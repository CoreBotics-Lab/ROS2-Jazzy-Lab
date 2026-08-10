#!/usr/bin/env bash
set -e

# ANSI Color Codes
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo -e "${CYAN}====================================================${NC}"
echo -e "${CYAN}  zenoh_ros — Master Library Setup & Dependencies  ${NC}"
echo -e "${CYAN}====================================================${NC}"

# 1. Run C++ setup (zenoh-c v1.9.0 & nlohmann/json + C++ nodes)
echo -e "\n${CYAN}[Step 1/3] Setting up C++ dependencies & PC node executables...${NC}"
CPP_INSTALL_SCRIPT="${SCRIPT_DIR}/cpp/zenoh_install.sh"
if [ -f "${CPP_INSTALL_SCRIPT}" ]; then
    bash "${CPP_INSTALL_SCRIPT}"
else
    echo -e "${RED}[ERROR] Could not find '${CPP_INSTALL_SCRIPT}'!${NC}"
    exit 1
fi

# 2. Run Python dependencies setup (pip install requirements.txt)
echo -e "\n${CYAN}[Step 2/3] Installing Python dependencies (eclipse-zenoh, msgpack, etc.)...${NC}"
PYTHON_REQ="${SCRIPT_DIR}/python/requirements.txt"
if [ -f "${PYTHON_REQ}" ]; then
    PIP_CMD=""
    if command -v pip3 &> /dev/null; then
        PIP_CMD="pip3"
    elif command -v pip &> /dev/null; then
        PIP_CMD="pip"
    else
        PIP_CMD="python3 -m pip"
    fi

    # Check if inside a virtualenv vs system Python (PEP 668 support)
    if [ -n "$VIRTUAL_ENV" ] || python3 -c "import sys; exit(0 if sys.prefix != sys.base_prefix else 1)" 2>/dev/null; then
        $PIP_CMD install -r "${PYTHON_REQ}"
    else
        $PIP_CMD install -r "${PYTHON_REQ}" --break-system-packages 2>/dev/null || $PIP_CMD install -r "${PYTHON_REQ}" --user 2>/dev/null || $PIP_CMD install -r "${PYTHON_REQ}"
    fi
    echo -e "${GREEN}[SUCCESS] Python dependencies installed.${NC}"
else
    echo -e "${YELLOW}[WARNING] Could not find '${PYTHON_REQ}'. Skipping Python pip install.${NC}"
fi

# 3. Run Echo CLI Tool setup (CMake build & ~/.bashrc alias setup)
echo -e "\n${CYAN}[Step 3/3] Setting up CLI Echo Debugger tool & ~/.bashrc alias...${NC}"
TOOLS_INSTALL_SCRIPT="${SCRIPT_DIR}/tools/echo_install.sh"
if [ -f "${TOOLS_INSTALL_SCRIPT}" ]; then
    bash "${TOOLS_INSTALL_SCRIPT}"
else
    echo -e "${RED}[ERROR] Could not find '${TOOLS_INSTALL_SCRIPT}'!${NC}"
    exit 1
fi

# 4. Source ~/.bashrc to load alias in current shell environment
if [ -f "${HOME}/.bashrc" ]; then
    # Disable non-interactive check if sourcing inside script
    source "${HOME}/.bashrc" 2>/dev/null || true
fi

echo -e "\n${GREEN}====================================================${NC}"
echo -e "${GREEN}  Master Setup Complete! All Components Ready.   ${NC}"
echo -e "${GREEN}====================================================${NC}"
echo -e "Alias '${CYAN}zenohEcho${NC}' has been registered and activated."
echo -e "Tip: You can also run setup directly with: ${CYAN}source shared_libraries/install.sh${NC}"
