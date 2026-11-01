from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():

    return LaunchDescription([

        # Turtlesim Simulator
        Node(
            package='turtlesim',
            executable='turtlesim_node',
            name='turtlesim',
            output='screen'
        ),

        # Goal Validator
        Node(
            package='turtle_navigator',
            executable='goal_validator',
            output='screen'
        ),

        # Navigation Action Server
        Node(
            package='turtle_navigator',
            executable='navigation_server',
            output='screen'
        ),

        # Results & Monitoring
        Node(
            package='turtle_navigator',
            executable='results_monitor',
            output='screen'
        ),

        # Qt Goal Input GUI
        Node(
            package='turtle_navigator',
            executable='goal_input',
            output='screen'
        ),

    ])
