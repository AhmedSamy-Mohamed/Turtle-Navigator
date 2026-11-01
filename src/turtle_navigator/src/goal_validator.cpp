#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"

#include "turtle_navigator_interfaces/srv/validate_goal.hpp"

class GoalValidator : public rclcpp::Node
{
public:
    GoalValidator()
    : Node("goal_validator")
    {
        // Declare a runtime-adjustable safety margin
        this->declare_parameter<double>("safety_margin", 1.0);

        // Create the validation service
        validation_service_ =
            this->create_service<
                turtle_navigator_interfaces::srv::ValidateGoal>(
                "validate_goal",
                std::bind(
                    &GoalValidator::validate_goal,
                    this,
                    std::placeholders::_1,
                    std::placeholders::_2
                )
            );

        RCLCPP_INFO(
            this->get_logger(),
            "Goal Validator is ready."
        );
    }

private:

    void validate_goal(
        const std::shared_ptr<
            turtle_navigator_interfaces::srv::ValidateGoal::Request> request,
        std::shared_ptr<
            turtle_navigator_interfaces::srv::ValidateGoal::Response> response)
    {
        // Read the current safety margin
        double margin =
            this->get_parameter("safety_margin").as_double();

        // Get target coordinates
        double x = request->goal.x;
        double y = request->goal.y;

        // Check whether the target is inside the safe area
        if (x < margin)
        {
            response->accepted = false;
            response->reason = "Goal is too close to the left wall.";
        }
        else if (x > 11.0 - margin)
        {
            response->accepted = false;
            response->reason = "Goal is too close to the right wall.";
        }
        else if (y < margin)
        {
            response->accepted = false;
            response->reason = "Goal is too close to the bottom wall.";
        }
        else if (y > 11.0 - margin)
        {
            response->accepted = false;
            response->reason = "Goal is too close to the top wall.";
        }
        else
        {
            response->accepted = true;
            response->reason = "Goal accepted.";
        }

        // Print the validation result
        if (response->accepted)
        {
            RCLCPP_INFO(
                this->get_logger(),
                "Goal accepted: x=%.2f, y=%.2f",
                x, y
            );
        }
        else
        {
            RCLCPP_WARN(
                this->get_logger(),
                "Goal rejected: %s",
                response->reason.c_str()
            );
        }
    }

    rclcpp::Service<
        turtle_navigator_interfaces::srv::ValidateGoal>::SharedPtr
        validation_service_;
};


int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);

    rclcpp::spin(
        std::make_shared<GoalValidator>()
    );

    rclcpp::shutdown();

    return 0;
}
