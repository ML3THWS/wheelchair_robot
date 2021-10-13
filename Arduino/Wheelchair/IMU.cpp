#include "IMU.h"



IMU::IMU() 
: imu("/imu/data", &imu_msg),
cali("/imu/cali", &cali_msg)
{}


void IMU::init(ros::NodeHandle& nh){

  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    nh.logwarn("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
  }
  
  //looking for Kalibration Data inside der EEPROM  
  EEPROM.get(eeAddress, bnoID);

  adafruit_bno055_offsets_t calibrationData;
  sensor_t sensor;

  bno.getSensor(&sensor);
  if (bnoID != sensor.sensor_id)
  {
    nh.loginfo("No Calibration Data for this sensor exists in EEPROM");
    delay(500);
  }
  else
  {
    //Serial.println("\nFound Calibration for this sensor in EEPROM.");
    eeAddress += sizeof(long);
    EEPROM.get(eeAddress, calibrationData);

    nh.loginfo("\n\nRestoring Calibration data to the BNO055...");
    bno.setSensorOffsets(calibrationData);

    nh.loginfo("\n\nCalibration data loaded into BNO055");
    foundCalib = true;
  }

  /*
   * Seting up the covariance over Parameter 
   * Askink if Parameters exists if not then use default values
  */
  nh.getParam("/serial_node/orientation_covariance", orientation_covariance, 9); //needs to be initialised once to work properly
  if (! nh.getParam("/serial_node/orientation_covariance", orientation_covariance, 9)) {
    //default Values
    orientation_covariance[0] = 0.01;
    orientation_covariance[1] = 0;
    orientation_covariance[2] = 0;

    orientation_covariance[3] = 0;
    orientation_covariance[4] = 0.01;
    orientation_covariance[5] = 0;

    orientation_covariance[6] = 0;
    orientation_covariance[7] = 0;
    orientation_covariance[8] = 0.01;
  }
  if (! nh.getParam("/serial_node/gyro_covariance", gyro_covariance, 9)) {
    //default Values
    gyro_covariance[0] = 0.1;
    gyro_covariance[0] = 0;
    gyro_covariance[0] = 0;
    
    gyro_covariance[0] = 0; 
    gyro_covariance[0] = 0.1;
    gyro_covariance[0] = 0;

    gyro_covariance[0] = 0;
    gyro_covariance[0] = 0;
    gyro_covariance[0] = 0.1;
  }
  if (! nh.getParam("/serial_node/linear_covariance", linear_covariance, 9)) {
    //default Values
    linear_covariance[0] = 0.5;
    linear_covariance[1] = 0;
    linear_covariance[2] = 0;
    
    linear_covariance[3] = 0;
    linear_covariance[4] = 0.5;
    linear_covariance[5] = 0;
    
    linear_covariance[6] = 0;
    linear_covariance[7] = 0;
    linear_covariance[8] = 0.5;
  }
  
  nh.initNode();
  nh.advertise(imu);
  nh.advertise(cali);
}

//call up IMU Data and publish it 
//Publishing Accelerometer, Gyro and Quaternion
void IMU::publish_imu_data(ros::NodeHandle& nh){
 
  // Possible vector values can be:
  // - VECTOR_ACCELEROMETER - m/s^2
  // - VECTOR_MAGNETOMETER  - uT
  // - VECTOR_GYROSCOPE     - °/s
  // - VECTOR_EULER         - degrees
  // - VECTOR_LINEARACCEL   - m/s^2
  // - VECTOR_GRAVITY       - m/s^2
  imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_ACCELEROMETER);
  imu::Vector<3> gyro = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);
  imu::Quaternion quat = bno.getQuat();

  // Header
  imu_msg.header.seq = seq;
  imu_msg.header.stamp = nh.now();
  imu_msg.header.frame_id = "base_link";

  // Quaternion data
  imu_msg.orientation.x = quat.x();
  imu_msg.orientation.y = quat.y();
  imu_msg.orientation.z = quat.z();
  imu_msg.orientation.w = quat.w();
  for (int i=0; i<9;i++){
    imu_msg.orientation_covariance[i] = orientation_covariance[i];
  }

  // Angular Velocity (transform degre to rad)
  imu_msg.angular_velocity.x = gyro.x()/57.296;
  imu_msg.angular_velocity.y = gyro.y()/57.296;
  imu_msg.angular_velocity.z = gyro.z()/57.296;
  for (int i=0; i<9;i++){
    imu_msg.angular_velocity_covariance[i] = gyro_covariance[i];
  }

  // linear acceleration
  imu_msg.linear_acceleration.x = accel.x();
  imu_msg.linear_acceleration.y = accel.y();
  imu_msg.linear_acceleration.z = accel.z(); 
  for (int i=0; i<9;i++){
    imu_msg.linear_acceleration_covariance[i] = linear_covariance[i];
  }
  
  imu.publish( &imu_msg );

  seq++;
  
}

//publish if IMU is fully Calibrated (best Case is 3 3 3)
void IMU::publish_imu_cali(){
  uint8_t system, cal_gyro, cal_accel, cal_mag = 0;
  bno.getCalibration(&system, &cal_gyro, &cal_accel, &cal_mag);
  cali_msg.x = cal_gyro;
  cali_msg.y = cal_accel;
  cali_msg.z = cal_mag;
  cali.publish( &cali_msg );
}
