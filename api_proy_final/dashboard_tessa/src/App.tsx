import { BrowserRouter, Routes, Route, Navigate } from "react-router-dom";
import DashboardPage from "./app/dashboard/page";
import ChartsPage from "./app/charts/page";
import ExamMonitorPage from "./app/exam-monitor/page";

import './App.css'

function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<Navigate to="/dashboard" replace />} />
        <Route path="/dashboard" element={<DashboardPage />} />
        <Route path="/charts" element={<ChartsPage />} />
        <Route path="/exam-monitor" element={<ExamMonitorPage />} />

      </Routes>
    </BrowserRouter>
  )
}

export default App
