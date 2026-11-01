# Limitations and Future Work

## 1. Introduction

Turtle Navigator is a ROS 2-based educational navigation project designed to demonstrate communication, control, and task execution using C++, Qt 6, and Turtlesim.

Although the project implements several navigation features, its current scope is intentionally limited to a simulated point-to-point navigation system.

This document describes the current limitations and possible future improvements.

## 2. Current Limitations

### 2.1 Simulation Environment

The current implementation operates using Turtlesim.

It has not been deployed or evaluated on a physical mobile robot.

Consequently, real-world performance, hardware reliability, and sensor accuracy have not been established.

### 2.2 No Obstacle Detection or Avoidance

The navigation controller does not detect or avoid obstacles.

It moves toward the requested target using the current turtle pose and a basic feedback control strategy.

Obstacle detection and avoidance are outside the current implementation scope.

### 2.3 No Global Path Planning

The system does not implement a global path planner.

It uses a direct point-to-point navigation approach rather than calculating an optimal route through a map.

### 2.4 Limited Motion Control

The navigation server uses a simple proportional angular controller and distance-dependent linear velocity.

It does not implement advanced control strategies such as PID-based motion control or model predictive control.

### 2.5 Goal Validation Scope

The goal validator checks whether the requested target lies within the permitted safety boundaries.

The safety margin is applied through the `/validate_goal` service.

The navigation action server independently checks that target coordinates are finite and within the basic Turtlesim coordinate range of `[0, 11]`.

However, the action server does not independently enforce the configurable safety margin.

Therefore, direct action requests that bypass the validation service do not receive the same safety-margin check.

### 2.6 Limited Localization

The system uses the pose information provided by Turtlesim.

It does not implement localization algorithms, sensor fusion, or map-based position estimation.

### 2.7 Testing Scope

Functional testing has been performed in simulation.

The project does not currently provide a complete automated unit-testing or integration-testing suite.

## 3. Future Improvements

The following improvements represent possible future development directions. They are not claimed as implemented features.

### 3.1 Advanced Navigation Algorithms

Possible improvements include:

- PID-based motion control.
- Improved velocity profiling.
- More advanced trajectory tracking.
- Better handling of navigation edge cases.

### 3.2 Path Planning

A future version could introduce path-planning algorithms such as:

- A*.
- Dijkstra's algorithm.
- Sampling-based planning approaches.

These algorithms could allow the robot to navigate through more complex environments.

### 3.3 Obstacle Detection and Avoidance

The system could be extended with simulated or physical distance sensors.

Potential additions include:

- LiDAR.
- Ultrasonic sensors.
- Depth cameras.

These sensors could support obstacle detection and local avoidance strategies.

### 3.4 Nav2 Integration

A future development stage could explore integration with the ROS 2 Navigation Stack (Nav2).

This would introduce more advanced navigation capabilities, including planning, localization integration, costmaps, and behavior-based navigation.

Such integration would require a broader architecture than the current custom navigation server.

### 3.5 Physical Robot Deployment

The project could be adapted to a differential-drive mobile robot.

This would require:

- Motor drivers.
- Wheel encoders.
- Robot-specific velocity interfaces.
- Sensor integration.
- Robot description and coordinate frames.
- Hardware calibration and testing.

### 3.6 Improved Validation and Testing

Future work could include:

- Automated unit tests.
- ROS 2 integration tests.
- More extensive boundary-condition testing.
- Controller performance measurements.
- Repeated experiments under different target positions.

### 3.7 GUI Improvements

The Qt 6 interface could be extended with:

- A live navigation visualization.
- Navigation history.
- Configurable controller parameters.
- More detailed error messages.
- Improved monitoring and status presentation.

## 4. Development Roadmap

| Stage | Planned Improvement |
|---|---|
| Stage 1 | Improve controller behavior and robustness |
| Stage 2 | Add automated testing |
| Stage 3 | Introduce obstacle detection and avoidance |
| Stage 4 | Explore global path planning |
| Stage 5 | Investigate Nav2 integration |
| Stage 6 | Adapt the system for physical robot deployment |

This roadmap represents possible development directions rather than a fixed implementation schedule.

## 5. Conclusion

Turtle Navigator provides a practical foundation for understanding ROS 2 communication, action-based task execution, feedback control, and Qt-based robot interfaces.

Its current implementation focuses on a clear and testable navigation workflow in simulation.

The identified limitations provide a structured direction for future development toward more advanced autonomous mobile robot systems.

---

