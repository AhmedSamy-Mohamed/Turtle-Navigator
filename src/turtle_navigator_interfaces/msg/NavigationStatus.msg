# Navigation states
uint8 IDLE=0
uint8 ACTIVE=1
uint8 SUCCEEDED=2
uint8 CANCELED=3
uint8 ABORTED=4

# Current navigation state
uint8 state

# Current turtle position
float64 current_x
float64 current_y
float64 current_theta

# Target position
float64 target_x
float64 target_y

# Navigation measurements
float64 remaining_distance
float64 total_distance
float64 elapsed_time
