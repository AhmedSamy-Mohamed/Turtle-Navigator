#include <chrono>
#include <memory>
#include <thread>
#include <mutex>
#include <cmath>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "turtle_navigator_interfaces/srv/validate_goal.hpp"
#include "turtle_navigator_interfaces/action/navigate_to_goal.hpp"

#include <QApplication>
#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMetaObject>

using namespace std::chrono_literals;

using ValidateGoal =
    turtle_navigator_interfaces::srv::ValidateGoal;

using NavigateAction =
    turtle_navigator_interfaces::action::NavigateToGoal;

using ClientGoalHandle =
    rclcpp_action::ClientGoalHandle<NavigateAction>;

class GoalInputWindow : public QWidget
{
public:
    GoalInputWindow()
    {
        node_ = std::make_shared<rclcpp::Node>("goal_input");

        validation_client_ =
            node_->create_client<ValidateGoal>("/validate_goal");

        action_client_ =
            rclcpp_action::create_client<NavigateAction>(
                node_, "/navigate_to_goal");

        setup_ui();

        executor_.add_node(node_);

        ros_thread_ = std::thread(
            [this]()
            {
                executor_.spin();
            });
    }

    ~GoalInputWindow() override
    {
        executor_.cancel();

        if (ros_thread_.joinable())
            ros_thread_.join();
    }

private:
    rclcpp::Node::SharedPtr node_;

    rclcpp::Client<ValidateGoal>::SharedPtr validation_client_;

    rclcpp_action::Client<NavigateAction>::SharedPtr action_client_;

    rclcpp::executors::SingleThreadedExecutor executor_;

    std::thread ros_thread_;

    QDoubleSpinBox *x_input_;
    QDoubleSpinBox *y_input_;
    QDoubleSpinBox *theta_input_;

    QCheckBox *use_theta_checkbox_;

    QPushButton *validate_button_;
    QPushButton *start_button_;
    QPushButton *cancel_button_;

    QLabel *status_label_;
    QLabel *remaining_label_;
    QLabel *distance_label_;
    QLabel *time_label_;

    bool validation_in_progress_ = false;

    void setup_ui()
    {
        setWindowTitle("Turtle Navigator");
        resize(480, 580);

        auto *main_layout = new QVBoxLayout(this);

        auto *title = new QLabel("Turtle Navigator");
        title->setStyleSheet(
            "font-size: 24px; font-weight: bold; color: #2563eb;");

        auto *subtitle = new QLabel(
            "ROS 2 Autonomous Navigation System");

        main_layout->addWidget(title);
        main_layout->addWidget(subtitle);

        // Target configuration
        auto *target_group = new QGroupBox("Navigation Target");
        auto *target_layout = new QFormLayout(target_group);

        x_input_ = new QDoubleSpinBox();
        y_input_ = new QDoubleSpinBox();
        theta_input_ = new QDoubleSpinBox();

        x_input_->setRange(0.0, 11.0);
        y_input_->setRange(0.0, 11.0);
        theta_input_->setRange(-3.14159, 3.14159);

        x_input_->setValue(5.0);
        y_input_->setValue(5.0);

        x_input_->setDecimals(2);
        y_input_->setDecimals(2);
        theta_input_->setDecimals(2);

        target_layout->addRow("Target X:", x_input_);
        target_layout->addRow("Target Y:", y_input_);
        target_layout->addRow("Target Theta:", theta_input_);

        use_theta_checkbox_ =
            new QCheckBox("Use final orientation");

        target_layout->addRow(use_theta_checkbox_);

        main_layout->addWidget(target_group);

        // Control buttons
        auto *button_layout = new QHBoxLayout();

        validate_button_ = new QPushButton("Validate");
        start_button_ = new QPushButton("Start Navigation");
        cancel_button_ = new QPushButton("Cancel");

        button_layout->addWidget(validate_button_);
        button_layout->addWidget(start_button_);
        button_layout->addWidget(cancel_button_);

        main_layout->addLayout(button_layout);

        // Monitoring
        auto *monitor_group =
            new QGroupBox("Navigation Monitoring");

        auto *monitor_layout =
            new QFormLayout(monitor_group);

        status_label_ = new QLabel("Idle");
        remaining_label_ = new QLabel("-- m");
        distance_label_ = new QLabel("-- m");
        time_label_ = new QLabel("-- s");

        monitor_layout->addRow("Status:", status_label_);
        monitor_layout->addRow(
            "Remaining Distance:", remaining_label_);
        monitor_layout->addRow(
            "Total Distance:", distance_label_);
        monitor_layout->addRow("Total Time:", time_label_);

        main_layout->addWidget(monitor_group);

        auto *footer = new QLabel(
            "Powered by ROS 2 and Qt 6");

        footer->setStyleSheet("color: gray;");
        main_layout->addWidget(footer);

        setStyleSheet(R"(
            QWidget {
                font-size: 14px;
            }

            QGroupBox {
                font-weight: bold;
                border: 1px solid #9ca3af;
                border-radius: 8px;
                margin-top: 10px;
                padding: 12px;
            }

            QPushButton {
                padding: 9px;
                border-radius: 5px;
                background-color: #2563eb;
                color: white;
            }

            QPushButton:hover {
                background-color: #1d4ed8;
            }
        )");

        connect(
            validate_button_,
            &QPushButton::clicked,
            this,
            [this]()
            {
                validate_goal(false);
            });

        connect(
            start_button_,
            &QPushButton::clicked,
            this,
            [this]()
            {
                validate_goal(true);
            });

        connect(
            cancel_button_,
            &QPushButton::clicked,
            this,
            [this]()
            {
                cancel_navigation();
            });
    }

    turtle_navigator_interfaces::msg::Goal get_current_goal()
    {
        turtle_navigator_interfaces::msg::Goal goal;

        goal.x = x_input_->value();
        goal.y = y_input_->value();
        goal.theta = theta_input_->value();
        goal.use_theta = use_theta_checkbox_->isChecked();

        return goal;
    }

    void validate_goal(bool start_after_validation)
    {
        if (validation_in_progress_)
            return;

        if (!validation_client_->service_is_ready())
        {
            status_label_->setText("Validator service unavailable");
            return;
        }

        auto request =
            std::make_shared<ValidateGoal::Request>();

        auto goal = get_current_goal();

        request->goal = goal;

        validation_in_progress_ = true;
        status_label_->setText("Validating goal...");

        validation_client_->async_send_request(
            request,
            [this, goal, start_after_validation](
                rclcpp::Client<ValidateGoal>::SharedFuture future)
            {
                auto response = future.get();

                QMetaObject::invokeMethod(
                    this,
                    [this, response, goal, start_after_validation]()
                    {
                        validation_in_progress_ = false;

                        if (!response->accepted)
                        {
                            status_label_->setText(
                                QString::fromStdString(response->reason));

                            return;
                        }

                        status_label_->setText("Goal accepted");

                        if (start_after_validation)
                            start_navigation(goal);
                    },
                    Qt::QueuedConnection);
            });
    }

    void start_navigation(
        const turtle_navigator_interfaces::msg::Goal &goal)
    {
        if (!action_client_->action_server_is_ready())
        {
            status_label_->setText("Navigation server unavailable");
            return;
        }

        NavigateAction::Goal action_goal;
        action_goal.target = goal;

        auto options =
            rclcpp_action::Client<NavigateAction>::SendGoalOptions();

        options.goal_response_callback =
            [this](const ClientGoalHandle::SharedPtr &goal_handle)
            {
                QMetaObject::invokeMethod(
                    this,
                    [this, goal_handle]()
                    {
                        if (goal_handle)
                        {
                            status_label_->setText("Navigating...");
                        }
                        else
                        {
                            status_label_->setText("Action goal rejected");
                        }
                    },
                    Qt::QueuedConnection);
            };

        options.feedback_callback =
            [this](
                ClientGoalHandle::SharedPtr,
                const std::shared_ptr<const NavigateAction::Feedback> feedback)
            {
                double remaining = feedback->remaining_distance;

                QMetaObject::invokeMethod(
                    this,
                    [this, remaining]()
                    {
                        remaining_label_->setText(
                            QString::number(remaining, 'f', 2) + " m");
                    },
                    Qt::QueuedConnection);
            };

        options.result_callback =
            [this](const ClientGoalHandle::WrappedResult &result)
            {
                QMetaObject::invokeMethod(
                    this,
                    [this, result]()
                    {
                        if (!result.result)
                        {
                            status_label_->setText("No result received");
                            return;
                        }

                        distance_label_->setText(
                            QString::number(
                                result.result->total_distance, 'f', 2)
                            + " m");

                        double elapsed =
                            result.result->total_time.sec +
                            result.result->total_time.nanosec / 1e9;

                        time_label_->setText(
                            QString::number(elapsed, 'f', 2) + " s");

                        if (result.code ==
                            rclcpp_action::ResultCode::SUCCEEDED)
                        {
                            status_label_->setText("Succeeded");
                        }
                        else if (result.code ==
                            rclcpp_action::ResultCode::CANCELED)
                        {
                            status_label_->setText("Canceled");
                        }
                        else
                        {
                            status_label_->setText("Aborted");
                        }
                    },
                    Qt::QueuedConnection);
            };

        remaining_label_->setText("-- m");
        distance_label_->setText("-- m");
        time_label_->setText("-- s");

        action_client_->async_send_goal(action_goal, options);
    }

    void cancel_navigation()
    {
        if (!action_client_->action_server_is_ready())
        {
            status_label_->setText("Navigation server unavailable");
            return;
        }

        action_client_->async_cancel_all_goals();

        status_label_->setText("Cancellation requested...");
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);

    QApplication app(argc, argv);

    int result;

    {
        GoalInputWindow window;
        window.show();

        result = app.exec();
    }

    rclcpp::shutdown();

    return result;
}
