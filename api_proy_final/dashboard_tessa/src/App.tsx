import { BrowserRouter, Routes, Route, Navigate } from "react-router-dom";
import DashboardPage from "./app/dashboard/page";
import ActionsPage from "./app/dashboard/actions/page";
import './App.css'

function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<Navigate to="/dashboard" replace />} />
        <Route path="/dashboard" element={<DashboardPage />} />
        <Route path="/dashboard/actions" element={<ActionsPage />} />
      </Routes>
    </BrowserRouter>
  )
}

export default App
