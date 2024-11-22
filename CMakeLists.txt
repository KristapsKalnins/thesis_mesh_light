#
# Copyright (c) 2019 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(mesh_light)

include_directories(
	${ZEPHYR_NRF_MODULE_DIR}/samples/bluetooth/mesh/common
)

zephyr_include_directories(${ZEPHYR_BASE}/subsys/bluetooth/mesh)

# NORDIC SDK APP START
target_sources(app PRIVATE
	src/main.c
	src/model_handler.c
	src/lib/model/prov_helper_srv.c
	src/lib/model/prov_helper_cli.c
	src/lib/prov_stage/provisioner_stage.c)
target_include_directories(app PRIVATE
	include
	${ZEPHYR_NRF_MODULE_DIR}/samples/bluetooth/mesh/common
)

target_include_directories(app PRIVATE
	include
	src/lib/model
)

target_include_directories(app PRIVATE
	include
	src/lib/prov_stage
)


# Preinitialization related to DFU over SMP for nRF52 series
if(CONFIG_SOC_SERIES_NRF52X)
  target_sources_ifdef(CONFIG_MCUMGR_TRANSPORT_BT app PRIVATE
    ${ZEPHYR_NRF_MODULE_DIR}/samples/bluetooth/mesh/common/smp_bt.c)
endif()

# NORDIC SDK APP END
