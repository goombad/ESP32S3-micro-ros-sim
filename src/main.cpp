#include <Arduino.h>
#include <micro_ros_platformio.h> //microros specific header, must be included before any other header

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

//create empty/uninitialized ROS objects
//can name whatever, just variable names
rcl_node_t node; 
rcl_publisher_t publisher;
rclc_support_t support; //rclc helper object,
rcl_allocator_t allocator; //ROS2 memory allocator
std_msgs__msg__Int32 msg; //message object, used to publish data

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
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "esp32_publisher");
  //&publisher pointer to publisher object, &node so publisher can reference node
  //creates ROS topic /esp32_publisher, with message type std_msgs/msg/Int32, should RCCHECK THIS
    msg.data = 0;

}

void loop() {
  msg.data++; //increment message data
  rcl_publish(&publisher, &msg, NULL); //publish message, should RCSOFTCHECK THIS
  
  delay(1000);
}