#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "turtle_navigator_interfaces/msg/navigation_status.hpp"

using namespace std::chrono_literals;

class ResultsMonitor : public rclcpp::Node
{
public:
    ResultsMonitor()
        : Node("results_monitor"),
          previous_state_(-1)
    {
        // Subscribe to navigation status updates
        status_subscriber_ =
            this->create_subscription<
                turtle_navigator_interfaces::msg::NavigationStatus>(
                "/navigation_status",
                rclcpp::QoS(10).transient_local(),
                std::bind(
                    &ResultsMonitor::status_callback,
                    this,
                    std::placeholders::_1));

        RCLCPP_INFO(
            this->get_logger(),
            "Results & Monitoring Node is ready.");
    }

private:

    rclcpp::Subscription<
        turtle_navigator_interfaces::msg::NavigationStatus>::SharedPtr
        status_subscriber_;

    int previous_state_;

    std::chrono::steady_clock::time_point last_update_time_;

    // Convert numerical state into readable text
    std::string get_state_name(uint8_t state)
    {
        switch (state)
        {
            case 0: return "IDLE";
            case 1: return "ACTIVE";
            case 2: return "SUCCEEDED";
            case 3: return "CANCELED";
            case 4: return "ABORTED";
            default: return "UNKNOWN";
        }
    }

    // Process incoming navigation status
    void status_callback(
        const turtle_navigator_interfaces::msg::NavigationStatus::SharedPtr msg)
    {
        auto now = std::chrono::steady_clock::now();

        bool state_changed = (msg->state != previous_state_);

        bool update_due =
            (previous_state_ == 1) &&
            (now - last_update_time_ >= 1s);

        if (!state_changed && !update_due)
        {
            return;
        }

        previous_state_ = msg->state;
        last_update_time_ = now;

        RCLCPP_INFO(
            this->get_logger(),
            "========================================");

        RCLCPP_INFO(
            this->get_logger(),
            "Navigation State: %s",
            get_state_name(msg->state).c_str());

        RCLCPP_INFO(
            this->get_logger(),
            "Current Position: X=%.2f, Y=%.2f",
            msg->current_x,
            msg->current_y);

        RCLCPP_INFO(
            this->get_logger(),
            "Target Position: X=%.2f, Y=%.2f",
            msg->target_x,
            msg->target_y);

        RCLCPP_INFO(
            this->get_logger(),
            "Remaining Distance: %.2f m",
            msg->remaining_distance);

        RCLCPP_INFO(
            this->get_logger(),
            "Total Distance: %.2f m",
            msg->total_distance);

        RCLCPP_INFO(
            this->get_logger(),
            "Elapsed Time: %.2f s",
            msg->elapsed_time);

        RCLCPP_INFO(
            this->get_logger(),
            "========================================");
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    rclcpp::spin(std::make_shared<ResultsMonitor>());

    rclcpp::shutdown();

    return 0;
}
