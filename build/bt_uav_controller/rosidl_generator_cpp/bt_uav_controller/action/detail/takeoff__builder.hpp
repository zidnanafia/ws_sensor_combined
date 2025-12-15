// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from bt_uav_controller:action/Takeoff.idl
// generated code does not contain a copyright notice

#ifndef BT_UAV_CONTROLLER__ACTION__DETAIL__TAKEOFF__BUILDER_HPP_
#define BT_UAV_CONTROLLER__ACTION__DETAIL__TAKEOFF__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "bt_uav_controller/action/detail/takeoff__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace bt_uav_controller
{

namespace action
{

namespace builder
{

class Init_Takeoff_Goal_target_altitude
{
public:
  Init_Takeoff_Goal_target_altitude()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::bt_uav_controller::action::Takeoff_Goal target_altitude(::bt_uav_controller::action::Takeoff_Goal::_target_altitude_type arg)
  {
    msg_.target_altitude = std::move(arg);
    return std::move(msg_);
  }

private:
  ::bt_uav_controller::action::Takeoff_Goal msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::bt_uav_controller::action::Takeoff_Goal>()
{
  return bt_uav_controller::action::builder::Init_Takeoff_Goal_target_altitude();
}

}  // namespace bt_uav_controller


namespace bt_uav_controller
{

namespace action
{

namespace builder
{

class Init_Takeoff_Result_success
{
public:
  Init_Takeoff_Result_success()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::bt_uav_controller::action::Takeoff_Result success(::bt_uav_controller::action::Takeoff_Result::_success_type arg)
  {
    msg_.success = std::move(arg);
    return std::move(msg_);
  }

private:
  ::bt_uav_controller::action::Takeoff_Result msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::bt_uav_controller::action::Takeoff_Result>()
{
  return bt_uav_controller::action::builder::Init_Takeoff_Result_success();
}

}  // namespace bt_uav_controller


namespace bt_uav_controller
{

namespace action
{

namespace builder
{

class Init_Takeoff_Feedback_current_altitude
{
public:
  Init_Takeoff_Feedback_current_altitude()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::bt_uav_controller::action::Takeoff_Feedback current_altitude(::bt_uav_controller::action::Takeoff_Feedback::_current_altitude_type arg)
  {
    msg_.current_altitude = std::move(arg);
    return std::move(msg_);
  }

private:
  ::bt_uav_controller::action::Takeoff_Feedback msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::bt_uav_controller::action::Takeoff_Feedback>()
{
  return bt_uav_controller::action::builder::Init_Takeoff_Feedback_current_altitude();
}

}  // namespace bt_uav_controller


namespace bt_uav_controller
{

namespace action
{

namespace builder
{

class Init_Takeoff_SendGoal_Request_goal
{
public:
  explicit Init_Takeoff_SendGoal_Request_goal(::bt_uav_controller::action::Takeoff_SendGoal_Request & msg)
  : msg_(msg)
  {}
  ::bt_uav_controller::action::Takeoff_SendGoal_Request goal(::bt_uav_controller::action::Takeoff_SendGoal_Request::_goal_type arg)
  {
    msg_.goal = std::move(arg);
    return std::move(msg_);
  }

private:
  ::bt_uav_controller::action::Takeoff_SendGoal_Request msg_;
};

class Init_Takeoff_SendGoal_Request_goal_id
{
public:
  Init_Takeoff_SendGoal_Request_goal_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Takeoff_SendGoal_Request_goal goal_id(::bt_uav_controller::action::Takeoff_SendGoal_Request::_goal_id_type arg)
  {
    msg_.goal_id = std::move(arg);
    return Init_Takeoff_SendGoal_Request_goal(msg_);
  }

private:
  ::bt_uav_controller::action::Takeoff_SendGoal_Request msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::bt_uav_controller::action::Takeoff_SendGoal_Request>()
{
  return bt_uav_controller::action::builder::Init_Takeoff_SendGoal_Request_goal_id();
}

}  // namespace bt_uav_controller


namespace bt_uav_controller
{

namespace action
{

namespace builder
{

class Init_Takeoff_SendGoal_Response_stamp
{
public:
  explicit Init_Takeoff_SendGoal_Response_stamp(::bt_uav_controller::action::Takeoff_SendGoal_Response & msg)
  : msg_(msg)
  {}
  ::bt_uav_controller::action::Takeoff_SendGoal_Response stamp(::bt_uav_controller::action::Takeoff_SendGoal_Response::_stamp_type arg)
  {
    msg_.stamp = std::move(arg);
    return std::move(msg_);
  }

private:
  ::bt_uav_controller::action::Takeoff_SendGoal_Response msg_;
};

class Init_Takeoff_SendGoal_Response_accepted
{
public:
  Init_Takeoff_SendGoal_Response_accepted()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Takeoff_SendGoal_Response_stamp accepted(::bt_uav_controller::action::Takeoff_SendGoal_Response::_accepted_type arg)
  {
    msg_.accepted = std::move(arg);
    return Init_Takeoff_SendGoal_Response_stamp(msg_);
  }

private:
  ::bt_uav_controller::action::Takeoff_SendGoal_Response msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::bt_uav_controller::action::Takeoff_SendGoal_Response>()
{
  return bt_uav_controller::action::builder::Init_Takeoff_SendGoal_Response_accepted();
}

}  // namespace bt_uav_controller


namespace bt_uav_controller
{

namespace action
{

namespace builder
{

class Init_Takeoff_GetResult_Request_goal_id
{
public:
  Init_Takeoff_GetResult_Request_goal_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::bt_uav_controller::action::Takeoff_GetResult_Request goal_id(::bt_uav_controller::action::Takeoff_GetResult_Request::_goal_id_type arg)
  {
    msg_.goal_id = std::move(arg);
    return std::move(msg_);
  }

private:
  ::bt_uav_controller::action::Takeoff_GetResult_Request msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::bt_uav_controller::action::Takeoff_GetResult_Request>()
{
  return bt_uav_controller::action::builder::Init_Takeoff_GetResult_Request_goal_id();
}

}  // namespace bt_uav_controller


namespace bt_uav_controller
{

namespace action
{

namespace builder
{

class Init_Takeoff_GetResult_Response_result
{
public:
  explicit Init_Takeoff_GetResult_Response_result(::bt_uav_controller::action::Takeoff_GetResult_Response & msg)
  : msg_(msg)
  {}
  ::bt_uav_controller::action::Takeoff_GetResult_Response result(::bt_uav_controller::action::Takeoff_GetResult_Response::_result_type arg)
  {
    msg_.result = std::move(arg);
    return std::move(msg_);
  }

private:
  ::bt_uav_controller::action::Takeoff_GetResult_Response msg_;
};

class Init_Takeoff_GetResult_Response_status
{
public:
  Init_Takeoff_GetResult_Response_status()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Takeoff_GetResult_Response_result status(::bt_uav_controller::action::Takeoff_GetResult_Response::_status_type arg)
  {
    msg_.status = std::move(arg);
    return Init_Takeoff_GetResult_Response_result(msg_);
  }

private:
  ::bt_uav_controller::action::Takeoff_GetResult_Response msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::bt_uav_controller::action::Takeoff_GetResult_Response>()
{
  return bt_uav_controller::action::builder::Init_Takeoff_GetResult_Response_status();
}

}  // namespace bt_uav_controller


namespace bt_uav_controller
{

namespace action
{

namespace builder
{

class Init_Takeoff_FeedbackMessage_feedback
{
public:
  explicit Init_Takeoff_FeedbackMessage_feedback(::bt_uav_controller::action::Takeoff_FeedbackMessage & msg)
  : msg_(msg)
  {}
  ::bt_uav_controller::action::Takeoff_FeedbackMessage feedback(::bt_uav_controller::action::Takeoff_FeedbackMessage::_feedback_type arg)
  {
    msg_.feedback = std::move(arg);
    return std::move(msg_);
  }

private:
  ::bt_uav_controller::action::Takeoff_FeedbackMessage msg_;
};

class Init_Takeoff_FeedbackMessage_goal_id
{
public:
  Init_Takeoff_FeedbackMessage_goal_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Takeoff_FeedbackMessage_feedback goal_id(::bt_uav_controller::action::Takeoff_FeedbackMessage::_goal_id_type arg)
  {
    msg_.goal_id = std::move(arg);
    return Init_Takeoff_FeedbackMessage_feedback(msg_);
  }

private:
  ::bt_uav_controller::action::Takeoff_FeedbackMessage msg_;
};

}  // namespace builder

}  // namespace action

template<typename MessageType>
auto build();

template<>
inline
auto build<::bt_uav_controller::action::Takeoff_FeedbackMessage>()
{
  return bt_uav_controller::action::builder::Init_Takeoff_FeedbackMessage_goal_id();
}

}  // namespace bt_uav_controller

#endif  // BT_UAV_CONTROLLER__ACTION__DETAIL__TAKEOFF__BUILDER_HPP_
