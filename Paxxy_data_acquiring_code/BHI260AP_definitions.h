/*
 * BHI260AP_Definitions.h
 *
 *  Created on: Feb 21, 2024
 *      Author: Thusitha Samarasekara
 */

#ifndef BHI260AP_DEFINITIONS_H_
#define BHI260AP_DEFINITIONS_H_


#include "mraa/gpio.h"

#define ENABLE_LACC_VS 0x01
#define ENABLE_ORI_VS 0x02

#define BHI_REG_READ_MASK 0x80

#define BHI_PRODUCT_ID                          UINT8_C(0x89)

/*! Register map */
#define BHI_REG_CHAN_CMD                        UINT8_C(0x00)
#define BHY2_REG_CHAN_FIFO_W                     UINT8_C(0x01)
#define BHY2_REG_CHAN_FIFO_NW                    UINT8_C(0x02)
#define BHY2_REG_CHAN_STATUS                     UINT8_C(0x03)
#define BHY2_REG_CHIP_CTRL                       UINT8_C(0x05)
#define BHY2_REG_HOST_INTERFACE_CTRL             UINT8_C(0x06)
#define BHI_REG_HOST_INTERRUPT_CTRL             UINT8_C(0x07)
#define BHY2_REG_RESET_REQ                       UINT8_C(0x14)
#define BHY2_REG_TIME_EV_REQ                     UINT8_C(0x15)
#define BHY2_REG_HOST_CTRL                       UINT8_C(0x16)
#define BHY2_REG_HOST_STATUS                     UINT8_C(0x17)
#define BHY2_REG_CRC_0                           UINT8_C(0x18)  /* Totally 4 */
#define BHI_REG_PRODUCT_ID                      UINT8_C(0x1C)
#define BHI_REG_REVISION_ID                     UINT8_C(0x1D)
#define BHY2_REG_ROM_VERSION_0                   UINT8_C(0x1E)  /* Totally 2 */
#define BHY2_REG_KERNEL_VERSION_0                UINT8_C(0x20)  /* Totally 2 */
#define BHY2_REG_USER_VERSION_0                  UINT8_C(0x22)  /* Totally 2 */
#define BHY2_REG_FEATURE_STATUS                  UINT8_C(0x24)
#define BHI_REG_BOOT_STATUS                     UINT8_C(0x25)
#define BHY2_REG_HOST_INTR_TIME_0                UINT8_C(0x26)  /* Totally 5 */
#define BHY2_REG_CHIP_ID                         UINT8_C(0x2B)
#define BHI_REG_INT_STATUS                      UINT8_C(0x2D)
#define BHY2_REG_ERROR_VALUE                     UINT8_C(0x2E)
#define BHY2_REG_ERROR_AUX                       UINT8_C(0x2F)
#define BHY2_REG_DEBUG_VALUE                     UINT8_C(0x30)
#define BHY2_REG_DEBUG_STATE                     UINT8_C(0x31)
#define BHY2_REG_GP_5                            UINT8_C(0x32)
#define BHY2_REG_GP_6                            UINT8_C(0x36)
#define BHY2_REG_GP_7                            UINT8_C(0x3A)


/*! Command packets */
#define BHY2_CMD_REQ_POST_MORTEM_DATA            UINT16_C(0x0001)
#define BHY2_CMD_UPLOAD_TO_PROGRAM_RAM           UINT16_C(0x0002)
#define BHY2_CMD_BOOT_PROGRAM_RAM                UINT16_C(0x0003)
#define BHY2_CMD_ERASE_FLASH                     UINT16_C(0x0004)
#define BHY2_CMD_WRITE_FLASH                     UINT16_C(0x0005)
#define BHY2_CMD_BOOT_FLASH                      UINT16_C(0x0006)
#define BHY2_CMD_SET_INJECT_MODE                 UINT16_C(0x0007)
#define BHY2_CMD_INJECT_DATA                     UINT16_C(0x0008)
#define BHY2_CMD_FIFO_FLUSH                      UINT16_C(0x0009)
#define BHY2_CMD_SW_PASSTHROUGH                  UINT16_C(0x000A)
#define BHY2_CMD_REQ_SELF_TEST                   UINT16_C(0x000B)
#define BHY2_CMD_REQ_FOC                         UINT16_C(0x000C)
#define BHI_CMD_CONFIG_SENSOR                   UINT16_C(0x000D)
#define BHY2_CMD_CHANGE_RANGE                    UINT16_C(0x000E)
#define BHY2_CMD_FIFO_FORMAT_CTRL                UINT16_C(0x0015)

/*! Boot status */
#define BHY2_BST_FLASH_DETECTED                  UINT8_C(0x01)
#define BHY2_BST_FLASH_VERIFY_DONE               UINT8_C(0x02)
#define BHY2_BST_FLASH_VERIFY_ERROR              UINT8_C(0x04)
#define BHY2_BST_NO_FLASH                        UINT8_C(0x08)
#define BHI_BST_HOST_INTERFACE_READY            UINT8_C(0x10)
#define BHY2_BST_HOST_FW_VERIFY_DONE             UINT8_C(0x20)
#define BHY2_BST_HOST_FW_VERIFY_ERROR            UINT8_C(0x40)
#define BHY2_BST_HOST_FW_IDLE                    UINT8_C(0x80)
#define BHY2_BST_CHECK_RETRY                     UINT8_C(100)

/*! Meta event definitions */
#define BHY2_META_EVENT_FLUSH_COMPLETE           (1)
#define BHY2_META_EVENT_SAMPLE_RATE_CHANGED      (2)
#define BHY2_META_EVENT_POWER_MODE_CHANGED       (3)
#define BHY2_META_EVENT_ALGORITHM_EVENTS         (5)
#define BHY2_META_EVENT_SENSOR_STATUS            (6)
#define BHY2_META_EVENT_BSX_DO_STEPS_MAIN        (7)
#define BHY2_META_EVENT_BSX_DO_STEPS_CALIB       (8)
#define BHY2_META_EVENT_BSX_GET_OUTPUT_SIGNAL    (9)
#define BHY2_META_EVENT_RESERVED1                (10)
#define BHY2_META_EVENT_SENSOR_ERROR             (11)
#define BHY2_META_EVENT_FIFO_OVERFLOW            (12)
#define BHY2_META_EVENT_DYNAMIC_RANGE_CHANGED    (13)
#define BHY2_META_EVENT_FIFO_WATERMARK           (14)
#define BHY2_META_EVENT_RESERVED2                (15)
#define BHI_META_EVENT_INITIALIZED              (16)
#define BHY2_META_TRANSFER_CAUSE                 (17)
#define BHY2_META_EVENT_SENSOR_FRAMEWORK         (18)
#define BHY2_META_EVENT_RESET                    (19)
#define BHI_META_EVENT_SPACER                   (20)

#define BHY2_SENSOR_ID_ACC_PASS                  UINT8_C(1)  /* Accelerometer passthrough */
#define BHY2_SENSOR_ID_ACC_RAW                   UINT8_C(3)  /* Accelerometer uncalibrated */
#define BHY2_SENSOR_ID_ACC                       UINT8_C(4)  /* Accelerometer corrected */
#define BHY2_SENSOR_ID_ACC_BIAS                  UINT8_C(5)  /* Accelerometer offset */
#define BHY2_SENSOR_ID_ACC_WU                    UINT8_C(6)  /* Accelerometer corrected wake up */
#define BHY2_SENSOR_ID_ACC_RAW_WU                UINT8_C(7)  /* Accelerometer uncalibrated wake up */
#define BHY2_SENSOR_ID_SI_ACCEL                  UINT8_C(8)  /* Virtual Sensor ID for Accelerometer */
#define BHY2_SENSOR_ID_GYRO_PASS                 UINT8_C(10) /* Gyroscope passthrough */
#define BHY2_SENSOR_ID_GYRO_RAW                  UINT8_C(12) /* Gyroscope uncalibrated */
#define BHY2_SENSOR_ID_GYRO                      UINT8_C(13) /* Gyroscope corrected */
#define BHY2_SENSOR_ID_GYRO_BIAS                 UINT8_C(14) /* Gyroscope offset */
#define BHY2_SENSOR_ID_GYRO_WU                   UINT8_C(15) /* Gyroscope wake up */
#define BHY2_SENSOR_ID_GYRO_RAW_WU               UINT8_C(16) /* Gyroscope uncalibrated wake up */
#define BHY2_SENSOR_ID_SI_GYROS                  UINT8_C(17) /* Virtual Sensor ID for Gyroscope */
#define BHY2_SENSOR_ID_MAG_PASS                  UINT8_C(19) /* Magnetometer passthrough */
#define BHY2_SENSOR_ID_MAG_RAW                   UINT8_C(21) /* Magnetometer uncalibrated */
#define BHY2_SENSOR_ID_MAG                       UINT8_C(22) /* Magnetometer corrected */
#define BHY2_SENSOR_ID_MAG_BIAS                  UINT8_C(23) /* Magnetometer offset */
#define BHY2_SENSOR_ID_MAG_WU                    UINT8_C(24) /* Magnetometer wake up */
#define BHY2_SENSOR_ID_MAG_RAW_WU                UINT8_C(25) /* Magnetometer uncalibrated wake up */
#define BHY2_SENSOR_ID_GRA                       UINT8_C(28) /* Gravity vector */
#define BHY2_SENSOR_ID_GRA_WU                    UINT8_C(29) /* Gravity vector wake up */
#define BHY2_SENSOR_ID_LACC                      UINT8_C(31) /* Linear acceleration */
#define BHI_SENSOR_ID_LACC_WU                   UINT8_C(32) /* Linear acceleration wake up */
#define BHY2_SENSOR_ID_RV                        UINT8_C(34) /* Rotation vector */
#define BHY2_SENSOR_ID_RV_WU                     UINT8_C(35) /* Rotation vector wake up */
#define BHY2_SENSOR_ID_GAMERV                    UINT8_C(37) /* Game rotation vector */
#define BHY2_SENSOR_ID_GAMERV_WU                 UINT8_C(38) /* Game rotation vector wake up */
#define BHY2_SENSOR_ID_GEORV                     UINT8_C(40) /* Geo-magnetic rotation vector */
#define BHY2_SENSOR_ID_GEORV_WU                  UINT8_C(41) /* Geo-magnetic rotation vector wake up */
#define BHY2_SENSOR_ID_ORI                       UINT8_C(43) /* Orientation */
#define BHI_SENSOR_ID_ORI_WU                    UINT8_C(44) /* Orientation wake up */
#define BHY2_SENSOR_ID_TILT_DETECTOR             UINT8_C(48) /* Tilt detector */
#define BHY2_SENSOR_ID_STD                       UINT8_C(50) /* Step detector */
#define BHY2_SENSOR_ID_STC                       UINT8_C(52) /* Step counter */
#define BHY2_SENSOR_ID_STC_WU                    UINT8_C(53) /* Step counter wake up */
#define BHY2_SENSOR_ID_SIG                       UINT8_C(55) /* Significant motion */
#define BHY2_SENSOR_ID_WAKE_GESTURE              UINT8_C(57) /* Wake gesture */
#define BHY2_SENSOR_ID_GLANCE_GESTURE            UINT8_C(59) /* Glance gesture */
#define BHY2_SENSOR_ID_PICKUP_GESTURE            UINT8_C(61) /* Pickup gesture */
#define BHY2_SENSOR_ID_AR                        UINT8_C(63) /* Activity recognition */
#define BHY2_SENSOR_ID_WRIST_TILT_GESTURE        UINT8_C(67) /* Wrist tilt gesture */
#define BHY2_SENSOR_ID_DEVICE_ORI                UINT8_C(69) /* Device orientation */
#define BHY2_SENSOR_ID_DEVICE_ORI_WU             UINT8_C(70) /* Device orientation wake up */
#define BHY2_SENSOR_ID_STATIONARY_DET            UINT8_C(75) /* Stationary detect */
#define BHY2_SENSOR_ID_MOTION_DET                UINT8_C(77) /* Motion detect */
#define BHY2_SENSOR_ID_ACC_BIAS_WU               UINT8_C(91) /* Accelerometer offset wake up */
#define BHY2_SENSOR_ID_GYRO_BIAS_WU              UINT8_C(92) /* Gyroscope offset wake up */
#define BHY2_SENSOR_ID_MAG_BIAS_WU               UINT8_C(93) /* Magnetometer offset wake up */
#define BHY2_SENSOR_ID_STD_WU                    UINT8_C(94) /* Step detector wake up */
#define BHY2_SENSOR_ID_TEMP                      UINT8_C(128) /* Temperature */
#define BHY2_SENSOR_ID_BARO                      UINT8_C(129) /* Barometer */
#define BHY2_SENSOR_ID_HUM                       UINT8_C(130) /* Humidity */
#define BHY2_SENSOR_ID_GAS                       UINT8_C(131) /* Gas */
#define BHY2_SENSOR_ID_TEMP_WU                   UINT8_C(132) /* Temperature wake up */
#define BHY2_SENSOR_ID_BARO_WU                   UINT8_C(133) /* Barometer wake up */
#define BHY2_SENSOR_ID_HUM_WU                    UINT8_C(134) /* Humidity wake up */
#define BHY2_SENSOR_ID_GAS_WU                    UINT8_C(135) /* Gas wake up */
#define BHY2_SENSOR_ID_STC_HW                    UINT8_C(136) /* Hardware Step counter */
#define BHY2_SENSOR_ID_STD_HW                    UINT8_C(137) /* Hardware Step detector */
#define BHY2_SENSOR_ID_SIG_HW                    UINT8_C(138) /* Hardware Significant motion */
#define BHY2_SENSOR_ID_STC_HW_WU                 UINT8_C(139) /* Hardware Step counter wake up */
#define BHY2_SENSOR_ID_STD_HW_WU                 UINT8_C(140) /* Hardware Step detector wake up */
#define BHY2_SENSOR_ID_SIG_HW_WU                 UINT8_C(141) /* Hardware Significant motion wake up */
#define BHY2_SENSOR_ID_ANY_MOTION                UINT8_C(142) /* Any motion */
#define BHY2_SENSOR_ID_ANY_MOTION_WU             UINT8_C(143) /* Any motion wake up */
#define BHY2_SENSOR_ID_EXCAMERA                  UINT8_C(144) /* External camera trigger */
#define BHY2_SENSOR_ID_GPS                       UINT8_C(145) /* GPS */
#define BHY2_SENSOR_ID_LIGHT                     UINT8_C(146) /* Light */
#define BHY2_SENSOR_ID_PROX                      UINT8_C(147) /* Proximity */
#define BHY2_SENSOR_ID_LIGHT_WU                  UINT8_C(148) /* Light wake up */
#define BHY2_SENSOR_ID_PROX_WU                   UINT8_C(149) /* Proximity wake up */
#define BHY2_SENSOR_ID_MULTI_TAP                 UINT8_C(153) /* Multi-tap detector (wear/hear) */
#define BHY2_SENSOR_ID_ACTIVITY                  UINT8_C(154) /* Activity recognition (wear/hear) */
#define BHY2_SENSOR_ID_WRIST_GEST                UINT8_C(156) /* Wrist gesture */
#define BHY2_SENSOR_ID_WRIST_WEAR_WU             UINT8_C(158) /* Wrist wear wake-up */
#define BHY2_SENSOR_ID_NO_MOTION                 UINT8_C(159) /* No motion detector */

/*! Physical sensor IDs*/
#define BHY2_PHYS_SENSOR_ID_ACCELEROMETER        UINT8_C(1)
#define BHY2_PHYS_SENSOR_ID_GYROSCOPE            UINT8_C(3)
#define BHY2_PHYS_SENSOR_ID_MAGNETOMETER         UINT8_C(5)
#define BHY2_PHYS_SENSOR_ID_PRESSURE             UINT8_C(11)
#define BHY2_PHYS_SENSOR_ID_HUMIDITY             UINT8_C(15)
#define BHY2_PHYS_SENSOR_ID_TEMPERATURE          UINT8_C(17)
#define BHY2_PHYS_SENSOR_ID_GAS_RESISTOR         UINT8_C(19)
#define BHY2_PHYS_SENSOR_ID_STEP_COUNTER         UINT8_C(32)
#define BHY2_PHYS_SENSOR_ID_STEP_DETECTOR        UINT8_C(33)
#define BHY2_PHYS_SENSOR_ID_ANY_MOTION           UINT8_C(35)
#define BHY2_PHYS_SENSOR_ID_ACTIVITY             UINT8_C(52)
#define BHY2_PHYS_SENSOR_ID_NO_MOTION            UINT8_C(55)
#define BHY2_PHYS_SENSOR_ID_WRIST_GESTURE        UINT8_C(56)
#define BHY2_PHYS_SENSOR_ID_WRIST_WAKEUP         UINT8_C(57)

/*! System data IDs */
#define BHI_IS_SYS_ID(sid)                      ((sid) >= 224)

#define BHI_SYS_ID_PADDING                      UINT8_C(0)
#define BHI_SYS_ID_TS_SMALL_DELTA               UINT8_C(251)
#define BHI_SYS_ID_TS_LARGE_DELTA               UINT8_C(252)
#define BHI_SYS_ID_TS_FULL                      UINT8_C(253)
#define BHI_SYS_ID_META_EVENT                   UINT8_C(254)
#define BHI_SYS_ID_TS_SMALL_DELTA_WU            UINT8_C(245)
#define BHI_SYS_ID_TS_LARGE_DELTA_WU            UINT8_C(246)
#define BHI_SYS_ID_TS_FULL_WU                   UINT8_C(247)
#define BHI_SYS_ID_META_EVENT_WU                UINT8_C(248)
#define BHI_SYS_ID_FILLER                       UINT8_C(255)
#define BHI_SYS_ID_DEBUG_MSG                    UINT8_C(250)
#define BHI_SYS_ID_BHI_LOG_UPDATE_SUB          UINT8_C(243)
#define BHI_SYS_ID_BHI_LOG_DOSTEP              UINT8_C(244)

/*! Interrupt control bits */
#define BHI_ICTL_DISABLE_FIFO_W                 UINT8_C(0x01)
#define BHI_ICTL_DISABLE_FIFO_NW                UINT8_C(0x02)
#define BHI_ICTL_DISABLE_STATUS_FIFO            UINT8_C(0x04)
#define BHI_ICTL_DISABLE_DEBUG                  UINT8_C(0x08)
#define BHI_ICTL_DISABLE_FAULT                  UINT8_C(0x10)
#define BHI_ICTL_ACTIVE_LOW                     UINT8_C(0x20)
#define BHI_ICTL_EDGE                           UINT8_C(0x40)
#define BHI_ICTL_OPEN_DRAIN                     UINT8_C(0x80)

//acceleration vector data
struct vector_data
{
	int16_t x;
	int16_t y;
	int16_t z;
};

//orientation euler data
struct euler_data
{
	int16_t heading;
	int16_t pitch;
	int16_t roll;
};

//BHI sensor class
struct BHI_sensor
{
	mraa_gpio_context cs;
	mraa_gpio_context drdy;
	uint8_t id;
	uint8_t data_ready;
	int8_t vector_ri;
	int8_t euler_ri;
	uint8_t initialized;
	uint8_t sensor_config;
	struct vector_data vector_buffer[2];
	struct euler_data euler_buffer[2];
	int vector_count;
	int euler_count;
};

//BHI interrupt status class
union BHI_interrupt_status
{
	struct
	{
		uint8_t host_int_asserted:1;
		uint8_t wake_up_fifo_status:2;
		uint8_t non_wake_up_fifo_status:2;
		uint8_t status:1;
		uint8_t debug:1;
		uint8_t reset_fault:1;
	}_;
	uint8_t ui8;
};


#endif /* BHI260AP_DEFINITIONS_H_ */
