import { SidebarProvider } from "@/components/ui/sidebar";
import { AppSidebar } from "@/components/app-sidebar";
import { ExamMonitor } from "@/components/exam-monitor";

export default function ExamMonitorPage() {
    return (
        <SidebarProvider>
            <AppSidebar />
            <main className="flex-1 p-6">
                <div className="max-w-7xl mx-auto">
                    <h1 className="text-3xl font-bold mb-6">Modo Examen</h1>
                    <ExamMonitor />
                </div>
            </main>
        </SidebarProvider>
    );
}
