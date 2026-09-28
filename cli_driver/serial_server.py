import asyncio
import json
import threading
import serial
import serial.tools.list_ports
import uvicorn
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.responses import HTMLResponse
import winreg
import itertools

app = FastAPI()

def get_all_com_ports():
    ports_dict = {}
    # 1. Quét qua thư viện pyserial
    try:
        for p in serial.tools.list_ports.comports():
            ports_dict[p.device] = p.description or p.device
    except Exception:
        pass
        
    # 2. Quét trực tiếp Windows Registry (bắt 100% cổng COM thực tế)
    try:
        key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r"HARDWARE\DEVICEMAP\SERIALCOMM")
        for i in itertools.count():
            try:
                val_name, port_name, _ = winreg.EnumValue(key, i)
                if port_name not in ports_dict:
                    ports_dict[port_name] = f"{port_name} ({val_name})"
            except OSError:
                break
        winreg.CloseKey(key)
    except Exception:
        pass

    result = [{"port": k, "desc": v} for k, v in sorted(ports_dict.items())]
    return result

# Quản lý Serial Port dạng Hub đa cổng (Multi-port, Multi-client)
class SerialHub:
    def __init__(self):
        self.ports = {}  # port_name: {"serial": Serial, "baud": int, "stop_event": Event, "clients": set(WebSocket)}
        self.client_port_map = {}  # websocket: port_name
        self.lock = threading.Lock()

    def get_client_port(self, ws: WebSocket):
        with self.lock:
            return self.client_port_map.get(ws)

    def connect_client(self, ws: WebSocket, port_name: str, baud: int, loop):
        with self.lock:
            # Rời khỏi cổng cũ nếu client này đang ở cổng khác
            old_port = self.client_port_map.get(ws)
            if old_port:
                self._remove_client_from_port(ws, old_port)

            # Nếu cổng COM này chưa được mở trên máy, mở mới và tạo luồng đọc
            if port_name not in self.ports:
                try:
                    sp = serial.Serial(port=port_name, baudrate=baud, timeout=0.1)
                    stop_event = threading.Event()
                    self.ports[port_name] = {
                        "serial": sp,
                        "baud": baud,
                        "stop_event": stop_event,
                        "clients": set(),
                        "thread": None
                    }
                    t = threading.Thread(target=self._port_reader_thread, args=(port_name, loop), daemon=True)
                    self.ports[port_name]["thread"] = t
                    t.start()
                except Exception as e:
                    raise e

            # Đăng ký client vào cổng này
            self.ports[port_name]["clients"].add(ws)
            self.client_port_map[ws] = port_name

    def disconnect_client(self, ws: WebSocket):
        with self.lock:
            port_name = self.client_port_map.get(ws)
            if port_name:
                self._remove_client_from_port(ws, port_name)
                del self.client_port_map[ws]

    def _remove_client_from_port(self, ws: WebSocket, port_name: str):
        if port_name in self.ports:
            self.ports[port_name]["clients"].discard(ws)
            # Nếu không còn tab nào dùng cổng này nữa, đóng cổng COM để giải phóng tài nguyên
            if len(self.ports[port_name]["clients"]) == 0:
                p_info = self.ports.pop(port_name)
                p_info["stop_event"].set()
                try:
                    p_info["serial"].close()
                except Exception:
                    pass

    def send_data(self, ws: WebSocket, data_str: str):
        with self.lock:
            port_name = self.client_port_map.get(ws)
            if port_name and port_name in self.ports:
                sp = self.ports[port_name]["serial"]
                if sp and sp.is_open:
                    sp.write(data_str.encode('utf-8'))

    def _port_reader_thread(self, port_name: str, loop):
        buffer = bytearray()
        while True:
            with self.lock:
                if port_name not in self.ports:
                    break
                p_info = self.ports[port_name]
                sp = p_info["serial"]
                stop_event = p_info["stop_event"]

            if stop_event.is_set():
                break

            try:
                if sp.is_open and sp.in_waiting > 0:
                    data = sp.read(sp.in_waiting)
                    buffer.extend(data)
                    while b'\n' in buffer:
                        line_bytes, buffer = buffer.split(b'\n', 1)
                        line = line_bytes.decode('utf-8', errors='replace').rstrip('\r')
                        if line:
                            is_log = line.strip().startswith('[')
                            msg = {
                                "type": "serial_line",
                                "text": line,
                                "is_log": is_log,
                                "port": port_name
                            }
                            with self.lock:
                                clients = list(self.ports.get(port_name, {}).get("clients", []))
                            for ws in clients:
                                if loop:
                                    asyncio.run_coroutine_threadsafe(ws.send_json(msg), loop)
                else:
                    threading.Event().wait(0.01)
            except Exception as e:
                # Báo lỗi mất kết nối cho các client đang cắm vào cổng này
                with self.lock:
                    clients = list(self.ports.get(port_name, {}).get("clients", []))
                    if port_name in self.ports:
                        p_info = self.ports.pop(port_name)
                        try:
                            p_info["serial"].close()
                        except Exception:
                            pass
                    for ws in clients:
                        if ws in self.client_port_map:
                            del self.client_port_map[ws]
                for ws in clients:
                    if loop:
                        asyncio.run_coroutine_threadsafe(
                            ws.send_json({"type": "status", "status": "disconnected", "msg": f"Cổng {port_name} bị ngắt: {str(e)}"}),
                            loop
                        )
                break

hub = SerialHub()
loop = None

HTML_CONTENT = """
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Serial Monitor &amp; CLI Controller</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <style>
        body { background-color: #0f172a; color: #e2e8f0; font-family: ui-monospace, monospace; }
        .log-tag-info { color: #4ade80; font-weight: bold; }
        .log-tag-warn { color: #facc15; font-weight: bold; }
        .log-tag-err  { color: #f87171; font-weight: bold; }
    </style>
</head>
<body class="h-screen flex flex-col p-4 gap-3">
    <!-- Thanh điều khiển trên cùng -->
    <header class="bg-slate-800 p-4 rounded-xl shadow-lg flex flex-wrap items-center justify-between gap-4 border border-slate-700">
        <!-- Option 1: Chọn cổng COM & Baudrate -->
        <div class="flex items-center gap-3">
            <span class="text-sm font-semibold text-slate-300">Cổng COM:</span>
            <select id="comPortSelect" class="bg-slate-900 border border-slate-600 rounded px-3 py-1.5 text-sm text-white focus:outline-none focus:border-blue-500">
                <option value="">Đang quét...</option>
            </select>
            <button onclick="fetchPorts()" class="bg-slate-700 hover:bg-slate-600 px-2.5 py-1.5 rounded text-xs text-slate-200">↻ Quét</button>

            <span class="text-sm font-semibold text-slate-300 ml-2">Baudrate:</span>
            <select id="baudSelect" class="bg-slate-900 border border-slate-600 rounded px-3 py-1.5 text-sm text-white">
                <option value="115200" selected>115200</option>
                <option value="9600">9600</option>
                <option value="19200">19200</option>
                <option value="38400">38400</option>
                <option value="57600">57600</option>
                <option value="921600">921600</option>
            </select>

            <button id="btnConnect" onclick="toggleConnect()" class="bg-emerald-600 hover:bg-emerald-500 text-white px-4 py-1.5 rounded text-sm font-medium transition-colors shadow">
                Kết Nối
            </button>
        </div>

        <!-- Option 2: Chọn Chế độ hiển thị & Xóa màn hình -->
        <div class="flex items-center gap-3">
            <span class="text-sm font-semibold text-slate-300">Chế độ hiển thị:</span>
            <select id="modeSelect" onchange="changeMode()" class="bg-slate-900 border border-indigo-500 rounded px-3 py-1.5 text-sm text-indigo-300 font-medium focus:outline-none">
                <option value="normal">Bình thường (CLI Only - Không Log)</option>
                <option value="log">Xem Log (Log Only - Bắt đầu bằng [...])</option>
            </select>

            <button onclick="clearTerminal()" class="bg-slate-700 hover:bg-slate-600 text-slate-300 px-3 py-1.5 rounded text-sm ml-2">
                Xóa màn hình
            </button>
        </div>
    </header>

    <!-- Màn hình Terminal / Log Viewer -->
    <main class="flex-1 bg-slate-950 rounded-xl p-4 border border-slate-800 overflow-y-auto font-mono text-sm leading-relaxed shadow-inner" id="terminalBox">
        <div class="text-slate-500 italic mb-2">[Hệ thống] Đã sẵn sàng. Vui lòng chọn cổng COM và bấm Kết Nối...</div>
    </main>

    <!-- Khu vực Nhập lệnh (Chỉ hiện ở chế độ Bình thường, tự động ẩn ở chế độ Log) -->
    <footer id="cliInputArea" class="bg-slate-800 p-3 rounded-xl shadow-lg flex gap-3 border border-slate-700 items-center">
        <span class="text-indigo-400 font-bold pl-2">&gt;</span>
        <input type="text" id="cmdInput" onkeydown="handleKeyDown(event)" placeholder="Nhập lệnh CLI rồi ấn Enter để gửi..." class="flex-1 bg-slate-900 border border-slate-700 rounded-lg px-4 py-2 text-sm text-white focus:outline-none focus:border-indigo-500 font-mono" autocomplete="off" />
        <button onclick="sendCommand()" class="bg-indigo-600 hover:bg-indigo-500 text-white px-6 py-2 rounded-lg text-sm font-medium transition-colors shadow">
            Gửi (Enter)
        </button>
    </footer>

    <script>
        let ws;
        let isConnected = false;

        function initWebSocket() {
            const protocol = location.protocol === 'https:' ? 'wss:' : 'ws:';
            ws = new WebSocket(`${protocol}//${location.host}/ws`);

            ws.onmessage = function(event) {
                const data = JSON.parse(event.data);
                if (data.type === "serial_line") {
                    appendSerialLine(data.text, data.is_log);
                } else if (data.type === "status") {
                    updateStatus(data.status, data.port, data.baud, data.msg);
                } else if (data.type === "port_list") {
                    renderPorts(data.ports);
                }
            };

            ws.onclose = function() {
                setTimeout(initWebSocket, 2000);
            };
        }

        function changeMode() {
            const mode = document.getElementById('modeSelect').value;
            const inputArea = document.getElementById('cliInputArea');
            if (mode === 'log') {
                inputArea.classList.add('hidden');
            } else {
                inputArea.classList.remove('hidden');
                document.getElementById('cmdInput').focus();
            }
        }

        function appendSerialLine(text, isLog) {
            const mode = document.getElementById('modeSelect').value;
            if (mode === 'log' && !isLog) return;
            if (mode === 'normal' && isLog) return;

            const box = document.getElementById('terminalBox');
            const lineDiv = document.createElement('div');
            lineDiv.className = 'py-0.5 whitespace-pre-wrap break-all';

            if (isLog) {
                let formatted = text
                    .replace(/\[INFO\]/g, '<span class="log-tag-info">[INFO]</span>')
                    .replace(/\[WARNING\]/g, '<span class="log-tag-warn">[WARNING]</span>')
                    .replace(/\[ERROR\]/g, '<span class="log-tag-err">[ERROR]</span>');
                lineDiv.innerHTML = formatted;
            } else {
                lineDiv.textContent = text;
                lineDiv.className += ' text-emerald-300';
            }

            box.appendChild(lineDiv);
            box.scrollTop = box.scrollHeight;
        }

        function clearTerminal() {
            document.getElementById('terminalBox').innerHTML = '';
        }

        function handleKeyDown(e) {
            if (e.key === 'Enter') {
                sendCommand();
            }
        }

        function sendCommand() {
            const input = document.getElementById('cmdInput');
            const cmd = input.value.trim();
            if (!cmd) return;
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ action: "send", command: cmd + "\\r\\n" }));
                input.value = '';
            }
        }

        function fetchPorts() {
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send(JSON.stringify({ action: "get_ports" }));
            }
        }

        function renderPorts(ports) {
            const select = document.getElementById('comPortSelect');
            const currentVal = select.value;
            select.innerHTML = '';
            if (ports.length === 0) {
                select.innerHTML = '<option value="">Không có cổng COM nào</option>';
                return;
            }
            ports.forEach(p => {
                const opt = document.createElement('option');
                opt.value = p.port;
                opt.textContent = `${p.port} (${p.desc})`;
                if (p.port === currentVal) opt.selected = true;
                select.appendChild(opt);
            });
        }

        function toggleConnect() {
            const port = document.getElementById('comPortSelect').value;
            const baud = parseInt(document.getElementById('baudSelect').value);
            if (!isConnected) {
                if (!port) { alert("Vui lòng chọn cổng COM!"); return; }
                ws.send(JSON.stringify({ action: "connect", port: port, baud: baud }));
            } else {
                ws.send(JSON.stringify({ action: "disconnect" }));
            }
        }

        function updateStatus(status, port, baud, msg) {
            const btn = document.getElementById('btnConnect');
            const selectPort = document.getElementById('comPortSelect');
            const selectBaud = document.getElementById('baudSelect');
            if (status === "connected") {
                isConnected = true;
                btn.textContent = "Ngắt Kết Nối";
                btn.className = "bg-rose-600 hover:bg-rose-500 text-white px-4 py-1.5 rounded text-sm font-medium transition-colors shadow";
                selectPort.disabled = true;
                selectBaud.disabled = true;
            } else {
                isConnected = false;
                btn.textContent = "Kết Nối";
                btn.className = "bg-emerald-600 hover:bg-emerald-500 text-white px-4 py-1.5 rounded text-sm font-medium transition-colors shadow";
                selectPort.disabled = false;
                selectBaud.disabled = false;
            }
            if (msg) {
                const box = document.getElementById('terminalBox');
                const line = document.createElement('div');
                line.className = 'text-yellow-400 italic py-1';
                line.textContent = `[Hệ thống] ${msg}`;
                box.appendChild(line);
                box.scrollTop = box.scrollHeight;
            }
        }

        window.onload = function() {
            initWebSocket();
            changeMode();
        };
    </script>
</body>
</html>
"""

@app.get("/")
async def get_index():
    return HTMLResponse(HTML_CONTENT)

@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await websocket.accept()
    ports = get_all_com_ports()
    await websocket.send_json({"type": "port_list", "ports": ports})

    try:
        while True:
            data_text = await websocket.receive_text()
            data = json.loads(data_text)
            action = data.get("action")

            if action == "get_ports":
                ports = get_all_com_ports()
                await websocket.send_json({"type": "port_list", "ports": ports})

            elif action == "connect":
                port = data.get("port")
                baud = data.get("baud", 115200)
                try:
                    hub.connect_client(websocket, port, baud, loop)
                    await websocket.send_json({
                        "type": "status",
                        "status": "connected",
                        "port": port,
                        "baud": baud,
                        "msg": f"Tab này đã kết nối tới {port} @ {baud} bps"
                    })
                except Exception as e:
                    await websocket.send_json({"type": "status", "status": "error", "msg": f"Lỗi mở {port}: {str(e)}"})

            elif action == "disconnect":
                hub.disconnect_client(websocket)
                await websocket.send_json({
                    "type": "status",
                    "status": "disconnected",
                    "msg": "Đã ngắt kết nối cổng COM cho tab này."
                })

            elif action == "send":
                cmd = data.get("command", "")
                hub.send_data(websocket, cmd)
    except WebSocketDisconnect:
        hub.disconnect_client(websocket)
    except Exception:
        hub.disconnect_client(websocket)

if __name__ == "__main__":
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    config = uvicorn.Config(app=app, host="0.0.0.0", port=11111, loop="asyncio")
    server = uvicorn.Server(config)
    loop.run_until_complete(server.serve())
