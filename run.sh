#!/bin/bash
# PillMate – Build & Run Script

set -e

echo "╔════════════════════════════════════════╗"
echo "║         PillMate – Build Script        ║"
echo "╚════════════════════════════════════════╝"
echo ""

# ── Backend ──
echo "▶ Building C backend..."
cd backend
gcc -Wall -o pillmate pillmate.c
echo "✅ Backend compiled successfully"
echo ""
echo "▶ Running backend (updates medicines.json)..."
./pillmate
echo ""
cd ..

# ── Frontend ──
echo "▶ Starting frontend server..."
echo "✅ Open http://localhost:3000 in your browser"
echo ""
cd frontend

# Try different servers
if command -v python3 &>/dev/null; then
  echo "Using Python HTTP server on port 3000"
  python3 -m http.server 3000
elif command -v python &>/dev/null; then
  python -m SimpleHTTPServer 3000
elif command -v npx &>/dev/null; then
  npx serve . -p 3000
else
  echo "⚠ No HTTP server found. Open frontend/index.html directly in browser."
fi
