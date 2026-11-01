#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "geometry_msgs/msg/twist.hpp"
#include "turtlesim_msgs/msg/pose.hpp"

#include "turtle_navigator_interfaces/action/navigate_to_goal.hpp"
#include "turtle_navigator_interfaces/msg/navigation_status.hpp"

using namespace std::chrono_literals;

class NavigationServer : public rclcpp::Node
{
public:
    using NavigateAction =
        turtle_navigator_interfaces::action::NavigateToGoal;

    using GoalHandle =
        rclcpp_action::ServerGoalHandle<NavigateAction>;

    NavigationServer()
    : Node("navigation_server")
    {
        // Publisher: sends velocity commands to turtlesim
        velocity_publisher_ =
            this->create_publisher<geometry_msgs::msg::Twist>(
                "/turtle1/cmd_vel", 10);

        // Publisher: publishes navigation status and measurements
        status_publisher_ =
            this->create_publisher<
                turtle_navigator_interfaces::msg::NavigationStatus>(
                "/navigation_status",
                rclcpp::QoS(10).transient_local());

        // Subscriber: receives the current turtle pose
        pose_subscriber_ =
            this->create_subscription<turtlesim_msgs::msg::Pose>(
                "/turtle1/pose",
                10,
                std::bind(
                    &NavigationServer::pose_callback,
                    this,
                    std::placeholders::_1));

        // Create the navigation action server
        action_server_ =
            rclcpp_action::create_server<NavigateAction>(
                this,
                "navigate_to_goal",

                std::bind(
                    &NavigationServer::handle_goal,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2),

                std::bind(
                    &NavigationServer::handle_cancel,
                    this,
                    std::placeholders::_1),

                std::bind(
                    &NavigationServer::handle_accepted,
                    this,
                    std::placeholders::_1));

        // Navigation control loop: 20 Hz
        control_timer_ = this->create_wall_timer(
            50ms,
            std::bind(
                &NavigationServer::control_loop,
                this));

        // Publish the initial IDLE navigation state
        navigation_state_ =
            turtle_navigator_interfaces::msg::NavigationStatus::IDLE;

        publish_navigation_status();

        RCLCPP_INFO(
            this->get_logger(),
            "Navigation Action Server is ready.");
    }

private:

    // Current turtle pose
    turtlesim_msgs::msg::Pose current_pose_;

    bool pose_received_ = false;
    bool previous_pose_valid_ = false;

    // Previous pose for distance calculation
    double previous_x_ = 0.0;
    double previous_y_ = 0.0;

    // Active navigation goal
    std::shared_ptr<GoalHandle> active_goal_;

    // Navigation measurements
    double total_distance_ = 0.0;
    double max_step_distance_ = 0.0;
    size_t pose_samples_ = 0;

    // Navigation monitoring data
    uint8_t navigation_state_ =
        turtle_navigator_interfaces::msg::NavigationStatus::IDLE;

    double target_x_ = 0.0;
    double target_y_ = 0.0;
    double remaining_distance_ = 0.0;
    double elapsed_time_ = 0.0;

    std::chrono::steady_clock::time_point start_time_;

    // ROS communication
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr
        velocity_publisher_;

    rclcpp::Subscription<turtlesim_msgs::msg::Pose>::SharedPtr
        pose_subscriber_;

    rclcpp::Publisher<
        turtle_navigator_interfaces::msg::NavigationStatus>::SharedPtr
        status_publisher_;

    rclcpp_action::Server<NavigateAction>::SharedPtr
        action_server_;

    rclcpp::TimerBase::SharedPtr control_timer_;


    // Publish current navigation status and measurements
    void publish_navigation_status()
    {
        auto message =
            turtle_navigator_interfaces::msg::NavigationStatus();

        message.state = navigation_state_;

        message.current_x = current_pose_.x;
        message.current_y = current_pose_.y;
        message.current_theta = current_pose_.theta;

        message.target_x = target_x_;
        message.target_y = target_y_;

        message.remaining_distance = remaining_distance_;
        message.total_distance = total_distance_;
        message.elapsed_time = elapsed_time_;

        status_publisher_->publish(message);
    }


    // Normalize an angle to [-pi, pi]
    double normalize_angle(double angle)
    {
        return std::atan2(std::sin(angle), std::cos(angle));
    }


    // Calculate distance between two points
    double calculate_distance(
        double x1, double y1,
        double x2, double y2)
    {
        return std::hypot(x2 - x1, y2 - y1);
    }


    // Stop the turtle
    void stop_turtle()
    {
        geometry_msgs::msg::Twist command;

        command.linear.x = 0.0;
        command.angular.z = 0.0;

        velocity_publisher_->publish(command);
    }


    // Receive turtle position
    void pose_callback(
        const turtlesim_msgs::msg::Pose::SharedPtr message)
    {
        current_pose_ = *message;
        pose_received_ = true;

        // Accumulate traveled distance only during navigation
        if (active_goal_ && previous_pose_valid_)
        {
            double step_distance = calculate_distance(
                previous_x_,
                previous_y_,
                current_pose_.x,
                current_pose_.y);

            total_distance_ += step_distance;
            max_step_distance_ = std::max(max_step_distance_, step_distance);
            pose_samples_++;
        }

        previous_x_ = current_pose_.x;
        previous_y_ = current_pose_.y;

        previous_pose_valid_ = true;
    }


    // Validate incoming action goal
    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID & uuid,
        std::shared_ptr<const NavigateAction::Goal> goal)
    {
        (void)uuid;

        double x = goal->target.x;
        double y = goal->target.y;
        double theta = goal->target.theta;

        if (!std::isfinite(x) ||
            !std::isfinite(y) ||
            !std::isfinite(theta))
        {
            RCLCPP_WARN(
                this->get_logger(),
                "Goal rejected: coordinates must be finite.");

            return rclcpp_action::GoalResponse::REJECT;
        }

        if (x < 0.0 || x > 11.0 ||
            y < 0.0 || y > 11.0)
        {
            RCLCPP_WARN(
                this->get_logger(),
                "Goal rejected: coordinates are outside turtlesim.");

            return rclcpp_action::GoalResponse::REJECT;
        }

        RCLCPP_INFO(
            this->get_logger(),
            "New goal received: x=%.2f, y=%.2f",
            x, y);

        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }


    // Accept cancellation requests
    rclcpp_action::CancelResponse handle_cancel(
        const std::shared_ptr<GoalHandle> goal_handle)
    {
        (void)goal_handle;

        RCLCPP_INFO(
            this->get_logger(),
            "Navigation cancellation requested.");

        return rclcpp_action::CancelResponse::ACCEPT;
    }


    // Handle an accepted goal
    void handle_accepted(
        const std::shared_ptr<GoalHandle> goal_handle)
    {
        // A new goal replaces the previous active goal
        if (active_goal_)
        {
            stop_turtle();

            auto old_result =
                std::make_shared<NavigateAction::Result>();

            old_result->success = false;
            old_result->total_distance = total_distance_;

            double elapsed =
                std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - start_time_
                ).count();

            builtin_interfaces::msg::Duration old_duration;
            old_duration.sec = static_cast<int32_t>(elapsed);
            old_duration.nanosec = static_cast<uint32_t>((elapsed - old_duration.sec) * 1e9);
            old_result->total_time = old_duration;

            // Publish the final state of the previous goal
            elapsed_time_ = elapsed;

            if (active_goal_->is_canceling())
            {
                navigation_state_ =
                    turtle_navigator_interfaces::msg::NavigationStatus::CANCELED;
            }
            else
            {
                navigation_state_ =
                    turtle_navigator_interfaces::msg::NavigationStatus::ABORTED;
            }

            if (pose_received_)
            {
                remaining_distance_ = std::hypot(
                    target_x_ - current_pose_.x,
                    target_y_ - current_pose_.y);
            }

            publish_navigation_status();

            // Finish the previous goal using the correct action transition.
            if (active_goal_->is_canceling())
            {
                active_goal_->canceled(old_result);

                RCLCPP_INFO(
                    this->get_logger(),
                    "Previous goal canceled before replacement.");
            }
            else
            {
                active_goal_->abort(old_result);

                RCLCPP_INFO(
                    this->get_logger(),
                    "Previous goal aborted and replaced by a new goal.");
            }
        }

        active_goal_ = goal_handle;

        total_distance_ = 0.0;
        max_step_distance_ = 0.0;
        pose_samples_ = 0;

        start_time_ = std::chrono::steady_clock::now();

        previous_pose_valid_ = pose_received_;

        if (pose_received_)
        {
            previous_x_ = current_pose_.x;
            previous_y_ = current_pose_.y;
        }

        // Initialize monitoring data for the new goal
        target_x_ = goal_handle->get_goal()->target.x;
        target_y_ = goal_handle->get_goal()->target.y;

        remaining_distance_ = pose_received_
            ? std::hypot(
                target_x_ - current_pose_.x,
                target_y_ - current_pose_.y)
            : 0.0;

        elapsed_time_ = 0.0;

        navigation_state_ =
            turtle_navigator_interfaces::msg::NavigationStatus::ACTIVE;

        publish_navigation_status();

        RCLCPP_INFO(
            this->get_logger(),
            "Navigation started.");
    }


    // Main navigation control loop
    void control_loop()
    {
        if (!active_goal_)
        {
            return;
        }

        // Handle cancellation
        if (active_goal_->is_canceling())
        {
            stop_turtle();

            auto result =
                std::make_shared<NavigateAction::Result>();

            result->success = false;
            result->total_distance = total_distance_;

            double elapsed =
                std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - start_time_
                ).count();

            builtin_interfaces::msg::Duration duration;
            duration.sec = static_cast<int32_t>(elapsed);
            duration.nanosec = static_cast<uint32_t>((elapsed - duration.sec) * 1e9);
            result->total_time = duration;

            elapsed_time_ = elapsed;
            navigation_state_ =
                turtle_navigator_interfaces::msg::NavigationStatus::CANCELED;

            publish_navigation_status();

            active_goal_->canceled(result);

            active_goal_.reset();
            previous_pose_valid_ = false;

            RCLCPP_INFO(
                this->get_logger(),
                "Navigation canceled.");

            return;
        }

        if (!pose_received_)
        {
            stop_turtle();
            return;
        }

        const auto goal = active_goal_->get_goal();

        double dx = goal->target.x - current_pose_.x;
        double dy = goal->target.y - current_pose_.y;

        double distance = std::hypot(dx, dy);

        // Update monitoring measurements
        remaining_distance_ = distance;

        elapsed_time_ =
            std::chrono::duration<double>(
                std::chrono::steady_clock::now() - start_time_
            ).count();

        navigation_state_ =
            turtle_navigator_interfaces::msg::NavigationStatus::ACTIVE;

        publish_navigation_status();

        // Desired direction toward the target
        double desired_angle = std::atan2(dy, dx);

        double angle_error =
            normalize_angle(desired_angle - current_pose_.theta);

        geometry_msgs::msg::Twist command;

        // Position tolerance
        const double position_tolerance = 0.10;

        // Orientation tolerance
        const double angle_tolerance = 0.05;

        if (distance > position_tolerance)
        {
            // Rotate first if the heading error is large
            if (std::abs(angle_error) > 0.35)
            {
                command.linear.x = 0.0;

                command.angular.z = std::clamp(
                    4.0 * angle_error,
                    -2.0,
                    2.0);
            }
            else
            {
                command.linear.x = std::min(
                    1.5,
                    1.5 * distance);

                command.angular.z = std::clamp(
                    4.0 * angle_error,
                    -2.0,
                    2.0);
            }
        }
        else if (goal->target.use_theta)
        {
            // Correct final orientation if requested
            double final_error = normalize_angle(
                goal->target.theta - current_pose_.theta);

            if (std::abs(final_error) > angle_tolerance)
            {
                command.linear.x = 0.0;

                command.angular.z = std::clamp(
                    4.0 * final_error,
                    -2.0,
                    2.0);
            }
            else
            {
                finish_goal();
                return;
            }
        }
        else
        {
            finish_goal();
            return;
        }

        // Publish velocity command
        velocity_publisher_->publish(command);

        // Send continuous action feedback
        auto feedback =
            std::make_shared<NavigateAction::Feedback>();

        feedback->remaining_distance = distance;

        active_goal_->publish_feedback(feedback);
    }


    // Finish navigation successfully
    void finish_goal()
    {
        stop_turtle();

        auto result =
            std::make_shared<NavigateAction::Result>();

        result->success = true;
        result->total_distance = total_distance_;

        double elapsed =
            std::chrono::duration<double>(
                std::chrono::steady_clock::now() - start_time_
            ).count();

        builtin_interfaces::msg::Duration duration;
        duration.sec = static_cast<int32_t>(elapsed);
        duration.nanosec = static_cast<uint32_t>((elapsed - duration.sec) * 1e9);
        result->total_time = duration;

        // Save final monitoring measurements
        remaining_distance_ = std::hypot(
            active_goal_->get_goal()->target.x - current_pose_.x,
            active_goal_->get_goal()->target.y - current_pose_.y);

        elapsed_time_ = elapsed;

        navigation_state_ =
            turtle_navigator_interfaces::msg::NavigationStatus::SUCCEEDED;

        publish_navigation_status();

        active_goal_->succeed(result);

        RCLCPP_INFO(
            this->get_logger(),
            "Goal reached successfully!");

        RCLCPP_INFO(
            this->get_logger(),
            "Total distance: %.3f",
            total_distance_);

        RCLCPP_INFO(
            this->get_logger(),
            "Pose samples: %zu | Maximum step distance: %.6f",
            pose_samples_,
            max_step_distance_);

        RCLCPP_INFO(
            this->get_logger(),
            "Total time: %.2f seconds",
            elapsed);

        active_goal_.reset();
        previous_pose_valid_ = false;
    }
};


int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);

    rclcpp::spin(
        std::make_shared<NavigationServer>());

    rclcpp::shutdown();

    return 0;
}
