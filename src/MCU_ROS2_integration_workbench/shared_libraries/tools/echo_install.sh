#!/usr/bin/env bash
set -e

# ANSI Color Codes
GREEN='\033[0;32m'
CYAN='\033[0;36m'
YELLOW='\033[0;33m'
RED='\033[0;31m'
NC='\033[0m'

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
ECHO_BIN="${BUILD_DIR}/echo"
BASHRC_FILE="${HOME}/.bashrc"
ALIAS_NAME="zenohEcho"

echo -e "${CYAN}==========================================${NC}"
echo -e "${CYAN}  zenoh_ros — Echo CLI Tool Installer    ${NC}"
echo -e "${CYAN}==========================================${NC}"

# 1. Check & Install Prerequisites (zenoh-c and json dependencies)
CPP_INSTALL_SCRIPT="${SCRIPT_DIR}/../cpp/zenoh_install.sh"
if [ -f "${CPP_INSTALL_SCRIPT}" ]; then
    echo -e "${CYAN}[1/3] Ensuring zenoh-c & C++ 3rdparty dependencies...${NC}"
    bash "${CPP_INSTALL_SCRIPT}"
fi

# 2. Check & Clean Stale CMake Cache (if copied from another workspace/path)
echo -e "${CYAN}[2/3] Preparing build environment & building echo CLI...${NC}"

if [ -f "${BUILD_DIR}/CMakeCache.txt" ]; then
    CACHE_SRC=$(grep "CMAKE_HOME_DIRECTORY:INTERNAL" "${BUILD_DIR}/CMakeCache.txt" 2>/dev/null | cut -d'=' -f2 || true)
    if [ "${CACHE_SRC}" != "${SCRIPT_DIR}" ]; then
        echo -e "${YELLOW}[NOTE] Cleaning stale CMake cache from previous directory path...${NC}"
        rm -rf "${BUILD_DIR}"
    fi
fi

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

if ! cmake "${SCRIPT_DIR}"; then
    echo -e "${YELLOW}[NOTE] CMake failed with existing cache. Re-building cleanly...${NC}"
    cd "${SCRIPT_DIR}"
    rm -rf "${BUILD_DIR}"
    mkdir -p "${BUILD_DIR}"
    cd "${BUILD_DIR}"
    cmake "${SCRIPT_DIR}"
fi

make echo -j$(nproc 2>/dev/null || echo 2)

if [ ! -f "${ECHO_BIN}" ]; then
    echo -e "${RED}[ERROR] Build failed! '${ECHO_BIN}' binary not found.${NC}"
    exit 1
fi
echo -e "${GREEN}[SUCCESS] Built '${ECHO_BIN}' binary.${NC}"

# 3. Dynamic Alias Check & Update in ~/.bashrc
echo -e "${CYAN}[3/3] Syncing alias '${ALIAS_NAME}' in ~/.bashrc...${NC}"

ALIAS_LINE="alias ${ALIAS_NAME}='\"${ECHO_BIN}\"'"

if grep -q "alias ${ALIAS_NAME}=" "${BASHRC_FILE}" 2>/dev/null; then
    # Check if existing alias points to the current ECHO_BIN path
    EXISTING_ALIAS=$(grep "alias ${ALIAS_NAME}=" "${BASHRC_FILE}" 2>/dev/null || true)
    if [[ "${EXISTING_ALIAS}" == *"${ECHO_BIN}"* ]]; then
        echo -e "${YELLOW}[NOTE] Alias '${ALIAS_NAME}' already points to current path in ~/.bashrc.${NC}"
    else
        echo -e "${YELLOW}[NOTE] Updating existing '${ALIAS_NAME}' alias to current binary path...${NC}"
        # Replace existing alias line using sed safely
        sed -i "/alias ${ALIAS_NAME}=/c\\${ALIAS_LINE}" "${BASHRC_FILE}"
        echo -e "${GREEN}[SUCCESS] Updated alias '${ALIAS_NAME}' in ~/.bashrc!${NC}"
    fi
else
    echo "" >> "${BASHRC_FILE}"
    echo "# zenoh_ros CLI Echo Tool Alias" >> "${BASHRC_FILE}"
    echo "${ALIAS_LINE}" >> "${BASHRC_FILE}"
    echo -e "${GREEN}[SUCCESS] Added alias '${ALIAS_NAME}' to ~/.bashrc!${NC}"
fi

echo -e "\n${GREEN}==========================================${NC}"
echo -e "${GREEN}  Installation Complete!                ${NC}"
echo -e "${GREEN}==========================================${NC}"
echo -e "To start using the alias in your current terminal run:"
echo -e "  ${CYAN}source ~/.bashrc${NC}"
echo -e "Then run:"
echo -e "  ${CYAN}${ALIAS_NAME} robot/hello_string 192.168.4.1${NC}"
echo -e "  ${CYAN}${ALIAS_NAME} robot/sim_counter 10.42.0.50 2547${NC}"
