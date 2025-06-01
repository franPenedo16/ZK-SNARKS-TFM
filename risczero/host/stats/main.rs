use methods::{
    GUEST_CODE_FOR_TFM_ELF
};
use risc0_zkvm::{default_prover, ExecutorEnv};

use json::parse;
use json_core::{Inputs};

use serde::Serialize;
use serde_json::to_string_pretty;
use std::{
    fs::File,
    io::{self, BufRead, BufReader, Write},
    sync::{
        atomic::{AtomicBool, Ordering},
        Arc, Mutex,
    },
    thread::{self, JoinHandle},
    time::{Duration, Instant},
};
use anyhow::Result;


#[derive(Clone, Serialize)]
struct Sample {
    timestamp: f64,
    kib: u64,
    percent: f64,
}

#[derive(Serialize)]
struct RunData {
    run: usize,
    samples: Vec<Sample>,
}

#[derive(Serialize)]
struct TimeData {
    run: usize,
    elapsed_secs: f64,
}

fn total_mem_kib() -> io::Result<u64> {
    let f = File::open("/proc/meminfo")?;
    for line in BufReader::new(f).lines() {
        let l = line?;
        if l.starts_with("MemTotal:") {
            let parts: Vec<_> = l.split_whitespace().collect();
            return Ok(parts[1].parse().unwrap());
        }
    }
    Err(io::Error::new(io::ErrorKind::Other, "MemTotal no encontrado"))
}

fn read_pss_kib() -> io::Result<u64> {
    let file = File::open("/proc/self/smaps")?;
    let reader = BufReader::new(file);

    let mut total = 0u64;
    for line in reader.lines() {
        let l = line?;
        if let Some(rest) = l.strip_prefix("Pss:") {
            if let Some(num_str) = rest.split_whitespace().next() {
                if let Ok(val) = num_str.parse::<u64>() {
                    total += val;
                }
            }
        }
    }
    Ok(total)
}

fn read_rss_kib() -> io::Result<u64> {
    let file = File::open("/proc/self/smaps")?;
    let reader = BufReader::new(file);

    let mut total = 0u64;
    for line in reader.lines() {
        let l = line?;
        if let Some(rest) = l.strip_prefix("Rss:") {
            // rest sería "               1234 kB"
            if let Some(num_str) = rest.split_whitespace().next() {
                if let Ok(val) = num_str.parse::<u64>() {
                    total += val;
                }
            }
        }
    }
    Ok(total)
}

fn spawn_sampler(
    interval: f64,
    total_mem: u64,
    stop_flag: Arc<AtomicBool>,
    samples_pss: Arc<Mutex<Vec<Sample>>>,
    samples_rss: Arc<Mutex<Vec<Sample>>>,
) -> JoinHandle<()> {
    thread::spawn(move || {
        let start = Instant::now();
        while !stop_flag.load(Ordering::SeqCst) {
            if let Ok(pss) = read_pss_kib() {
                let elapsed = start.elapsed().as_secs_f64();
                let perc = (pss as f64 / total_mem as f64) * 100.0;
                let sample = Sample {
                    timestamp: (elapsed * 1_000.0).round() / 1_000.0,
                    kib: pss,
                    percent: (perc * 100.0).round() / 100.0,
                };
                samples_pss.lock().unwrap().push(sample);
            }
            if let Ok(rss) = read_rss_kib() {
                let elapsed = start.elapsed().as_secs_f64();
                let perc = (rss as f64 / total_mem as f64) * 100.0;
                let sample = Sample {
                    timestamp: (elapsed * 1_000.0).round() / 1_000.0,
                    kib: rss,
                    percent: (perc * 100.0).round() / 100.0,
                };
                samples_rss.lock().unwrap().push(sample);
            }
            thread::sleep(Duration::from_secs_f64(interval));
        }
    })
}

fn main() -> Result<()> {
    let interval = 30.0;
    let warmups = 5;
    let reps = 30;
    let total_mem = total_mem_kib()?;

    let mut all_runs_pss: Vec<Vec<Sample>> = Vec::new();
    let mut all_runs_rss: Vec<Vec<Sample>> = Vec::new();
    let mut all_times: Vec<f64> = Vec::new();

    let data = include_str!("../../res/input.json");

    let data = parse(&data).unwrap();

    let mut data_array_sol: [[u8; 9]; 9] = [[0; 9]; 9];
    let mut data_array_prob: [[u8; 9]; 9] = [[0; 9]; 9];

    for i in 0..9 {
        for j in 0..9 {
            data_array_prob[i][j] = data["problem"][i][j].as_u8().unwrap();
            data_array_sol[i][j] = data["solution"][i][j].as_u8().unwrap();
        }
    }

    let input = Inputs {
        dni: data["dni"].to_string(),
        password: data["password"].to_string(),
        problem: data_array_prob,
        solution: data_array_sol,
    };

    for run in 1..=(warmups + reps) {
        let is_warmup = run <= warmups;
        println!(
            "===> Running test #{} {}",
            run,
            if is_warmup { "(warmup)" } else { "" }
        );

        let stop_flag = Arc::new(AtomicBool::new(false));
        let samples_pss: Arc<Mutex<Vec<Sample>>> = Arc::new(Mutex::new(Vec::new()));
        let samples_rss: Arc<Mutex<Vec<Sample>>> = Arc::new(Mutex::new(Vec::new()));
        let sampler_handle = spawn_sampler(interval, total_mem, Arc::clone(&stop_flag), Arc::clone(&samples_pss), Arc::clone(&samples_rss));

        let env = ExecutorEnv::builder()
        .write(&input)
        .unwrap()
        .build()
        .unwrap();

        let start = Instant::now();
        default_prover()
            .prove(env, GUEST_CODE_FOR_TFM_ELF)
            .unwrap();
        let elapsed = start.elapsed().as_secs_f64();

        stop_flag.store(true, Ordering::SeqCst);
        sampler_handle.join().unwrap();

        if !is_warmup {
            let run_idx = run - warmups;
            println!(
                "    Run #{}: total time = {:.3}s",
                run_idx,
                elapsed,
            );
            all_runs_pss.push(samples_pss.lock().unwrap().clone());
            all_runs_rss.push(samples_rss.lock().unwrap().clone());
            all_times.push((elapsed * 1_000.0).round() / 1_000.0);
        }
    }

    let run_data_pss: Vec<RunData> = all_runs_pss
    .into_iter()
    .enumerate()
    .map(|(idx, samples)| RunData {
        run: idx + 1,
        samples,
    })
    .collect();

    let run_data_rss: Vec<RunData> = all_runs_rss
    .into_iter()
    .enumerate()
    .map(|(idx, samples)| RunData {
        run: idx + 1,
        samples,
    })
    .collect();

    let time_data: Vec<TimeData> = all_times
    .into_iter()
    .enumerate()
    .map(|(idx, elapsed)| TimeData {
        run: idx + 1,
        elapsed_secs: elapsed,
    })
    .collect();

    
    let mem_json = to_string_pretty(&run_data_pss)?;
    let mut mem_file = File::create("stats/memory_pss_usage.json")?;
    mem_file.write_all(mem_json.as_bytes())?;
    println!("PSS saved in memory_pss_usage.json");

    let mem_json = to_string_pretty(&run_data_rss)?;
    let mut mem_file = File::create("stats/memory_rss_usage.json")?;
    mem_file.write_all(mem_json.as_bytes())?;
    println!("RSS saved in memory_rss_usage.json");

    let time_json = to_string_pretty(&time_data)?;
    let mut time_file = File::create("stats/execution_times.json")?;
    time_file.write_all(time_json.as_bytes())?;
    println!("Times save in execution_times.json");

    Ok(())
}


