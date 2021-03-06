#-------------------------------------------------
#
# Project created by QtCreator 2016-08-23T15:35:07
#
#-------------------------------------------------

QT += core gui charts serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = plan-gui
TEMPLATE = app
DEFINES += BOARD_TYPE_FLEXSEA_PLAN INCLUDE_UPROJ_RICNU_KNEE_V1
DEFINES += INCLUDE_UPROJ_MIT_A2DOF INCLUDE_UPROJ_CYCLE_TESTER
DEFINES += INCLUDE_UPROJ_ACTPACK

INCLUDEPATH += inc \
				inc/MDIWinObjects \
				inc/FlexSEADevice \
				flexsea-comm/inc \
				flexsea-system/inc \
				flexsea-shared/unity \
				flexsea-shared/inc \
				flexsea-projects/inc \
				flexsea-projects/Rigid/inc \
				flexsea-projects/MIT_2DoF_Ankle_v1/inc \
				flexsea-projects/RICNU_Knee_v1/inc \
				flexsea-projects/ActPack/inc

SOURCES += src/main.cpp \
				src/mainwindow.cpp \
				src/datalogger.cpp \
				src/flexsea_generic.cpp \
				src/serialdriver.cpp \
				src/flexsea_board.c \
				src/trapez.c \
				src/peripherals.c \
				src/commanager.cpp \
				src/dataprovider.cpp \
				src/FlexSEADevice/executeDevice.cpp \
				src/FlexSEADevice/flexseaDevice.cpp \
				src/FlexSEADevice/batteryDevice.cpp \
				src/FlexSEADevice/gossipDevice.cpp \
				src/FlexSEADevice/manageDevice.cpp \
				src/FlexSEADevice/strainDevice.cpp \
				src/FlexSEADevice/ricnuProject.cpp \
				src/FlexSEADevice/ankle2DofProject.cpp \
				src/MDIWinObjects/w_2dplot.cpp \
				src/MDIWinObjects/w_anycommand.cpp \
				src/MDIWinObjects/w_battery.cpp \
				src/MDIWinObjects/w_calibration.cpp \
				src/MDIWinObjects/w_control.cpp \
				src/MDIWinObjects/w_converter.cpp \
				src/MDIWinObjects/w_execute.cpp \
				src/MDIWinObjects/w_gossip.cpp \
				src/MDIWinObjects/w_manage.cpp \
				src/MDIWinObjects/w_ricnu.cpp \
				src/MDIWinObjects/w_slavecomm.cpp \
				src/MDIWinObjects/w_strain.cpp \
				src/MDIWinObjects/w_config.cpp \
				src/MDIWinObjects/w_logkeypad.cpp \
				src/MDIWinObjects/w_userrw.cpp \
				src/MDIWinObjects/w_incontrol.cpp \
				src/MDIWinObjects/w_event.cpp \
				src/MDIWinObjects/w_commtest.cpp \
				flexsea-shared/unity/unity.c \
				flexsea-projects/src/flexsea_cmd_user.c \
				flexsea-projects/src/dynamic_user_structs_plan.c \
				flexsea-projects/src/flexsea_user_structs.c \
				flexsea-projects/MIT_2DoF_Ankle_v1/src/cmd-MIT_2DoF_Ankle_v1.c \
				flexsea-projects/RICNU_Knee_v1/src/cmd-RICNU_Knee_v1.c \
				flexsea-comm/src/flexsea.c \
				flexsea-comm/src/flexsea_buffers.c \
				flexsea-comm/src/flexsea_circular_buffer.c \
				flexsea-comm/src/flexsea_comm.c \
				flexsea-comm/src/flexsea_payload.c \
				flexsea-system/src/flexsea_system.c \
				flexsea-system/src/flexsea_global_structs.c \
				flexsea-system/src/flexsea_cmd_data.c \
				flexsea-system/src/flexsea_cmd_external.c \
				flexsea-system/src/flexsea_cmd_sensors.c \
				flexsea-system/src/flexsea_cmd_calibration.c \
				flexsea-system/src/flexsea_cmd_control_1.c \
				flexsea-system/src/flexsea_cmd_control_2.c \
				flexsea-system/src/flexsea_cmd_tools.c \
				flexsea-system/src/flexsea_cmd_in_control.c \
				flexsea-system/src/flexsea_cmd_stream.c \
				flexsea-projects/MIT_2DoF_Ankle_v1/src/user-ex-MIT_2DoF_Ankle_v1.c \
				flexsea-projects/MIT_2DoF_Ankle_v1/src/user-mn-MIT_2DoF_Ankle_v1.c \
				flexsea-projects/RICNU_Knee_v1/src/user-ex-RICNU_Knee_v1.c \
				flexsea-projects/RICNU_Knee_v1/src/user-mn-RICNU_Knee_v1.c \
				flexsea-projects/ActPack/src/cmd-ActPack.c \
				flexsea-projects/src/user-ex.c \
				flexsea-projects/src/user-mn.c \
				src/FlexSEADevice/rigidDevice.cpp \
				src/MDIWinObjects/w_rigid.cpp \
				flexsea-projects/Rigid/src/cmd-Rigid.c \
				src/dynamicuserdatamanager.cpp \
				flexsea-projects/src/dynamic_user_structs_common.c \
				src/MDIWinObjects/w_status.cpp

HEADERS += inc/main.h \
				inc/mainwindow.h \
				inc/counter.h \
				inc/datalogger.h \
				inc/trapez.h \
				inc/serialdriver.h \
				inc/flexsea_generic.h \
				inc/flexsea_board.h \
				inc/peripherals.h \
				inc/commanager.h \
				inc/define.h \
				inc/dataprovider.h \
				inc/FlexSEADevice/executeDevice.h \
				inc/FlexSEADevice/flexseaDevice.h \
				inc/FlexSEADevice/batteryDevice.h \
				inc/FlexSEADevice/gossipDevice.h \
				inc/FlexSEADevice/manageDevice.h \
				inc/FlexSEADevice/strainDevice.h \
				inc/FlexSEADevice/ricnuProject.h \
				inc/FlexSEADevice/ankle2DofProject.h \
				inc/FlexSEADevice/rigidDevice.h \
				inc/MDIWinObjects/w_2dplot.h \
				inc/MDIWinObjects/w_anycommand.h \
				inc/MDIWinObjects/w_battery.h \
				inc/MDIWinObjects/w_calibration.h \
				inc/MDIWinObjects/w_control.h \
				inc/MDIWinObjects/w_converter.h \
				inc/MDIWinObjects/w_execute.h \
				inc/MDIWinObjects/w_gossip.h \
				inc/MDIWinObjects/w_manage.h \
				inc/MDIWinObjects/w_ricnu.h \
				inc/MDIWinObjects/w_slavecomm.h \
				inc/MDIWinObjects/w_strain.h \
				inc/MDIWinObjects/w_incontrol.h \
				inc/MDIWinObjects/w_event.h \
				inc/MDIWinObjects/w_config.h \
				inc/MDIWinObjects/w_logkeypad.h \
				inc/MDIWinObjects/w_userrw.h \
				inc/MDIWinObjects/w_commtest.h \
				flexsea-shared/unity/unity.h \
				flexsea-shared/unity/unity_internals.h \
				flexsea-projects/inc/flexsea_cmd_user.h \
				flexsea-projects/MIT_2DoF_Ankle_v1/inc/cmd-MIT_2DoF_Ankle_v1.h \
				flexsea-projects/RICNU_Knee_v1/inc/cmd-RICNU_Knee_v1.h \
				flexsea-projects/inc/flexsea_user_structs.h \
				flexsea-projects/inc/dynamic_user_structs.h \
				flexsea-comm/inc/flexsea.h \
				flexsea-comm/inc/flexsea_comm_def.h \
				flexsea-comm/inc/flexsea_buffers.h \
				flexsea-comm/inc/flexsea_circular_buffer.h \
				flexsea-comm/inc/flexsea_comm.h \
				flexsea-comm/inc/flexsea_payload.h \
				flexsea-system/inc/flexsea_system.h \
				flexsea-system/inc/flexsea_sys_def.h \
				flexsea-system/inc/flexsea_global_structs.h \
				flexsea-system/inc/flexsea_dataformats.h \
				flexsea-system/inc/flexsea_cmd_data.h \
				flexsea-system/inc/flexsea_cmd_external.h \
				flexsea-system/inc/flexsea_cmd_sensors.h \
				flexsea-system/inc/flexsea_cmd_calibration.h \
				flexsea-system/inc/flexsea_cmd_control.h \
				flexsea-system/inc/flexsea_cmd_tools.h \
				flexsea-system/inc/flexsea_cmd_in_control.h \
				inc/MDIWinObjects/w_rigid.h \
				flexsea-projects/Rigid/inc/cmd-Rigid.h \
				inc/MDIWinObjects/w_event.h \
				flexsea-system/inc/flexsea_cmd_stream.h \
				flexsea-system/inc/flexsea_dataformats.h \
				inc/dynamicuserdatamanager.h \
				flexsea-projects/ActPack/inc/cmd-ActPack.h \
				flexsea-system/test/flexsea-system_test-all.h \
				inc/MDIWinObjects/w_status.h

FORMS += ui/mainwindow.ui \
				ui/w_execute.ui \
				ui/w_control.ui \
				ui/w_2dplot.ui \
				ui/w_slavecomm.ui \
				ui/w_anycommand.ui \
				ui/w_converter.ui \
				ui/w_ricnu.ui \
				ui/w_manage.ui \
				ui/w_calibration.ui \
				ui/w_battery.ui \
				ui/w_gossip.ui \
				ui/w_strain.ui \
				ui/w_config.ui \
				ui/w_logkeypad.ui \
				ui/w_userrw.ui \
				ui/w_commtest.ui \
				ui/w_incontrol.ui \
				ui/w_rigid.ui \
				ui/w_event.ui \
				ui/w_status.ui

RESOURCES += \
	misc/icons.qrc

QMAKE_CFLAGS = $$QMAKE_CFLAGS -Wno-unused-but-set-variable

#Enable the next 2 lines to inspect the pre-processor output
#Linked will fail - debug only
#QMAKE_CFLAGS = $$QMAKE_CFLAGS -E
#QMAKE_CXXFLAGS = $$QMAKE_CXXFLAGS -E

DISTFILES += \
	flexsea-projects/flexsea.gitignore \
	flexsea-projects/GPL-3.0.txt \
	flexsea-projects/README.md \
	flexsea-comm/flexsea.gitignore \
	flexsea-comm/GPL-3.0.txt \
	flexsea-shared/unity/readme.txt \
	flexsea-shared/flexsea.gitignore \
	flexsea-shared/GPL-3.0.txt \
	flexsea-system/.gitignore \
	flexsea-system/GPL-3.0.txt \
	.gitignore \
	.gitmodules \
	GPL-3.0.txt \
	flexsea-comm/README.md \
	flexsea-shared/README.md \
	flexsea-system/README.md \
	README.md
