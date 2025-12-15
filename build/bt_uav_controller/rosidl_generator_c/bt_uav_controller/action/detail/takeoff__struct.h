// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from bt_uav_controller:action/Takeoff.idl
// generated code does not contain a copyright notice

#ifndef BT_UAV_CONTROLLER__ACTION__DETAIL__TAKEOFF__STRUCT_H_
#define BT_UAV_CONTROLLER__ACTION__DETAIL__TAKEOFF__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in action/Takeoff in the package bt_uav_controller.
typedef struct bt_uav_controller__action__Takeoff_Goal
{
  float target_altitude;
} bt_uav_controller__action__Takeoff_Goal;

// Struct for a sequence of bt_uav_controller__action__Takeoff_Goal.
typedef struct bt_uav_controller__action__Takeoff_Goal__Sequence
{
  bt_uav_controller__action__Takeoff_Goal * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} bt_uav_controller__action__Takeoff_Goal__Sequence;


// Constants defined in the message

/// Struct defined in action/Takeoff in the package bt_uav_controller.
typedef struct bt_uav_controller__action__Takeoff_Result
{
  bool success;
} bt_uav_controller__action__Takeoff_Result;

// Struct for a sequence of bt_uav_controller__action__Takeoff_Result.
typedef struct bt_uav_controller__action__Takeoff_Result__Sequence
{
  bt_uav_controller__action__Takeoff_Result * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} bt_uav_controller__action__Takeoff_Result__Sequence;


// Constants defined in the message

/// Struct defined in action/Takeoff in the package bt_uav_controller.
typedef struct bt_uav_controller__action__Takeoff_Feedback
{
  float current_altitude;
} bt_uav_controller__action__Takeoff_Feedback;

// Struct for a sequence of bt_uav_controller__action__Takeoff_Feedback.
typedef struct bt_uav_controller__action__Takeoff_Feedback__Sequence
{
  bt_uav_controller__action__Takeoff_Feedback * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} bt_uav_controller__action__Takeoff_Feedback__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
#include "unique_identifier_msgs/msg/detail/uuid__struct.h"
// Member 'goal'
#include "bt_uav_controller/action/detail/takeoff__struct.h"

/// Struct defined in action/Takeoff in the package bt_uav_controller.
typedef struct bt_uav_controller__action__Takeoff_SendGoal_Request
{
  unique_identifier_msgs__msg__UUID goal_id;
  bt_uav_controller__action__Takeoff_Goal goal;
} bt_uav_controller__action__Takeoff_SendGoal_Request;

// Struct for a sequence of bt_uav_controller__action__Takeoff_SendGoal_Request.
typedef struct bt_uav_controller__action__Takeoff_SendGoal_Request__Sequence
{
  bt_uav_controller__action__Takeoff_SendGoal_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} bt_uav_controller__action__Takeoff_SendGoal_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'stamp'
#include "builtin_interfaces/msg/detail/time__struct.h"

/// Struct defined in action/Takeoff in the package bt_uav_controller.
typedef struct bt_uav_controller__action__Takeoff_SendGoal_Response
{
  bool accepted;
  builtin_interfaces__msg__Time stamp;
} bt_uav_controller__action__Takeoff_SendGoal_Response;

// Struct for a sequence of bt_uav_controller__action__Takeoff_SendGoal_Response.
typedef struct bt_uav_controller__action__Takeoff_SendGoal_Response__Sequence
{
  bt_uav_controller__action__Takeoff_SendGoal_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} bt_uav_controller__action__Takeoff_SendGoal_Response__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.h"

/// Struct defined in action/Takeoff in the package bt_uav_controller.
typedef struct bt_uav_controller__action__Takeoff_GetResult_Request
{
  unique_identifier_msgs__msg__UUID goal_id;
} bt_uav_controller__action__Takeoff_GetResult_Request;

// Struct for a sequence of bt_uav_controller__action__Takeoff_GetResult_Request.
typedef struct bt_uav_controller__action__Takeoff_GetResult_Request__Sequence
{
  bt_uav_controller__action__Takeoff_GetResult_Request * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} bt_uav_controller__action__Takeoff_GetResult_Request__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'result'
// already included above
// #include "bt_uav_controller/action/detail/takeoff__struct.h"

/// Struct defined in action/Takeoff in the package bt_uav_controller.
typedef struct bt_uav_controller__action__Takeoff_GetResult_Response
{
  int8_t status;
  bt_uav_controller__action__Takeoff_Result result;
} bt_uav_controller__action__Takeoff_GetResult_Response;

// Struct for a sequence of bt_uav_controller__action__Takeoff_GetResult_Response.
typedef struct bt_uav_controller__action__Takeoff_GetResult_Response__Sequence
{
  bt_uav_controller__action__Takeoff_GetResult_Response * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} bt_uav_controller__action__Takeoff_GetResult_Response__Sequence;


// Constants defined in the message

// Include directives for member types
// Member 'goal_id'
// already included above
// #include "unique_identifier_msgs/msg/detail/uuid__struct.h"
// Member 'feedback'
// already included above
// #include "bt_uav_controller/action/detail/takeoff__struct.h"

/// Struct defined in action/Takeoff in the package bt_uav_controller.
typedef struct bt_uav_controller__action__Takeoff_FeedbackMessage
{
  unique_identifier_msgs__msg__UUID goal_id;
  bt_uav_controller__action__Takeoff_Feedback feedback;
} bt_uav_controller__action__Takeoff_FeedbackMessage;

// Struct for a sequence of bt_uav_controller__action__Takeoff_FeedbackMessage.
typedef struct bt_uav_controller__action__Takeoff_FeedbackMessage__Sequence
{
  bt_uav_controller__action__Takeoff_FeedbackMessage * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} bt_uav_controller__action__Takeoff_FeedbackMessage__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // BT_UAV_CONTROLLER__ACTION__DETAIL__TAKEOFF__STRUCT_H_
