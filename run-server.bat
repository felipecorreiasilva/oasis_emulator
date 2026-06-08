@echo off
setlocal
cd /d "%~dp0"
echo Iniciando os servidores Oasis...

if not exist "%~dp0login-server.exe" (
    echo ERRO: login-server.exe nao encontrado em %~dp0
) else (
    start "Login Server" "%~dp0login-server.exe"
)

if not exist "%~dp0char-server.exe" (
    echo AVISO: char-server.exe nao encontrado em %~dp0. Compile o projeto src\char antes.
) else (
    start "Char Server" "%~dp0char-server.exe"
)

if not exist "%~dp0map-server.exe" (
    echo AVISO: map-server.exe nao encontrado em %~dp0. Compile o projeto src\map antes.
) else (
    start "Map Server" "%~dp0map-server.exe"
)

necho Servidores iniciados. Feche esta janela para encerrar.
pause
endlocal
