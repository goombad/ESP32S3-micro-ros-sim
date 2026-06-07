#include <Arduino.h>
#include <micro_ros_platformio.h> //microros specific header, must be included before any other header
#include <Wire.h> //for I2C
#include <Adafruit_MPU6050.h> //for MPU6050 IMU sensor
#include <Adafruit_Sensor.h> //for Adafruit sensor library, used by MPU6050 library

#include <rcl/rcl.h> //ROS CLIENT LIBRARY
//rcl specific header, must be included after microros_platformio.h
//core ROS2 objects

//allows rcl_node_t, rcl_publisher_t, rcl_subscription_t, rcl_timer_t, etc. to be used in the code
#include <rcl/error_handling.h> 
//RCCHECK and RCSOFTCHECK macros, used to return values of rcl functions
#include <rclc/rclc.h> //ROS Client Library for C, use for embedded/microcontroller development

#include <rclc/executor.h> //rclc specific header, must be included after rcl.h
//allows rclc_executor_t, rclc_executor_add_subscription(), rclc_executor
//checks for events and callbacks, functions that get called on event

#include <std_msgs/msg/int32.h> //message type header
//setting NODES to only communicate in 32int ROS messages
//include other message types as needed, e.g. std_msgs/msg/string.h for string messages

#include <sensor_msgs/msg/imu.h> //creates imu message type

Adafruit_MPU6050 mpu; //create MPU6050 object

#define IMU_SCL_PIN 47
#define IMU_SDA_PIN 21
#define MPU6050_ADDR 0x68

//create empty/uninitialized ROS objects
//can name whatever, just variable names
rcl_node_t node; 
rcl_publisher_t publisher;
rclc_support_t support; //rclc helper object,
rcl_allocator_t allocator; //ROS2 memory allocator
sensor_msgs__msg__Imu imu_msg; //message object, used to publish data

// Error loop: board gets stuck here if a critical ROS setup step fails
void error_loop() {
  while (1) {
    delay(100);
  }
}

// Hard check: use for setup/init functions that must succeed
#define RCCHECK(fn) { \
  rcl_ret_t temp_rc = fn; \
  if ((temp_rc != RCL_RET_OK)) { \
    error_loop(); \
  } \
}

// Soft check: use for runtime functions where one failure is not fatal
#define RCSOFTCHECK(fn) { \
  rcl_ret_t temp_rc = fn; \
  (void)temp_rc; \
}


void setup() {
  Serial.begin(115200);
  sensor_msgs__msg__Imu__init(&imu_msg); //initialize object imu_msg, do not assume header files are zeroed out
  imu_msg.orientation_covariance[0] = -1; 

  Wire.begin(IMU_SDA_PIN, IMU_SCL_PIN); //sda first, scl second

  if (!mpu.begin(MPU6050_ADDR)) {
    Serial.println("MPU6050 i2c initialization failed!");
    error_loop();
  }
  delay(1000);

  set_microros_serial_transports(Serial); //sets transport layer to use USB serial
  allocator = rcl_get_default_allocator(); //choose memory allocator

  rclc_support_init(&support, 0, NULL, &allocator); //should RCCHECK THIS
  //&support and &allocator pointer to objects
  //initialize support using default allocator, no args
  //this means support object can reference allocator object and use it for memory management

  rclc_node_init_default(&node, "esp32_node", "", &support); //creates node "esp32_node", should RCCHECK THIS
  //&node pointer to node object, "esp32_node" is name of node
  //&support so node can reference support

  rclc_publisher_init_default(
    &publisher,
    &node, //attach publisher to node
    ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Imu),
    "imu/data");
  //&publisher pointer to publisher object, &node so publisher can reference node
  //creates ROS topic /imu/data, with message type sensor_msgs/msg/Imu, should RCCHECK THIS


}

void loop() {
  sensors_event_t accel, gyro, temp; //struct type sensors_event_t can hold data from any type of sensor
  //accel gyro temp are empty objects to fill data
  mpu.getEvent(&accel, &gyro, &temp); //fills accel, gyro, temp with sensor data, rewrites it every iteration

  float ax = accel.acceleration.x; //get x acceleration in m/s^2
  float ay = accel.acceleration.y; //get y acceleration in m/s^2
  float az = accel.acceleration.z; //get z acceleration in m/s^2

  float gx = gyro.gyro.x; //get x gyro in rad/s
  float gy = gyro.gyro.y; //get y gyro in rad/s
  float gz = gyro.gyro.z; //get z gyro in rad/s

  //so that node /esp32_node can send ROS messages to topic /imu/data and publish it
  imu_msg.linear_acceleration.x = ax; //fill imu message with accel data
  imu_msg.linear_acceleration.y = ay; 
  imu_msg.linear_acceleration.z = az;
  imu_msg.angular_velocity.x = gx; //fill imu message with gyro data
  imu_msg.angular_velocity.y = gy;
  imu_msg.angular_velocity.z = gz;
  rcl_publish(&publisher, &imu_msg, NULL);
  delay(50);
}