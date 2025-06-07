import axios from 'axios'
import router from '@/router'

const api = axios.create({
    baseURL: 'http://localhost:3500/api/',
    headers: { 'Content-Type': 'application/json' }
})

api.interceptors.request.use(config => {
    const token = localStorage.getItem('token')
    if (token) {
        config.headers.Authorization = `Bearer ${token}`
    }
    return config
})

api.interceptors.response.use(
    res => { return { res, ok: true } },
    err => {
        if (err.response && (err.response.status === 401 || err.response.status === 403)) {
            localStorage.removeItem('token')
            router.push({ path: '/login' })
        }
        return { code: err.response.status, ok: false }
    }
)

export default api