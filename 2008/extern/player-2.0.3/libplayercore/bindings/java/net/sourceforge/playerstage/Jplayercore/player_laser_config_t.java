/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 1.3.24
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

package net.sourceforge.playerstage.Jplayercore;

public class player_laser_config_t {
  private long swigCPtr;
  protected boolean swigCMemOwn;

  protected player_laser_config_t(long cPtr, boolean cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = cPtr;
  }

  protected static long getCPtr(player_laser_config_t obj) {
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  protected void finalize() {
    delete();
  }

  public void delete() {
    if(swigCPtr != 0 && swigCMemOwn) {
      swigCMemOwn = false;
      playercore_javaJNI.delete_player_laser_config_t(swigCPtr);
    }
    swigCPtr = 0;
  }

  protected static long[] cArrayUnwrap(player_laser_config_t[] arrayWrapper) {
      long[] cArray = new long[arrayWrapper.length];
      for (int i=0; i<arrayWrapper.length; i++)
        cArray[i] = player_laser_config_t.getCPtr(arrayWrapper[i]);
      return cArray;
  }

  protected static player_laser_config_t[] cArrayWrap(long[] cArray, boolean cMemoryOwn) {
    player_laser_config_t[] arrayWrapper = new player_laser_config_t[cArray.length];
    for (int i=0; i<cArray.length; i++)
      arrayWrapper[i] = new player_laser_config_t(cArray[i], cMemoryOwn);
    return arrayWrapper;
  }

  public void setMin_angle(float min_angle) {
    playercore_javaJNI.set_player_laser_config_t_min_angle(swigCPtr, min_angle);
  }

  public float getMin_angle() {
    return playercore_javaJNI.get_player_laser_config_t_min_angle(swigCPtr);
  }

  public void setMax_angle(float max_angle) {
    playercore_javaJNI.set_player_laser_config_t_max_angle(swigCPtr, max_angle);
  }

  public float getMax_angle() {
    return playercore_javaJNI.get_player_laser_config_t_max_angle(swigCPtr);
  }

  public void setResolution(float resolution) {
    playercore_javaJNI.set_player_laser_config_t_resolution(swigCPtr, resolution);
  }

  public float getResolution() {
    return playercore_javaJNI.get_player_laser_config_t_resolution(swigCPtr);
  }

  public void setMax_range(float max_range) {
    playercore_javaJNI.set_player_laser_config_t_max_range(swigCPtr, max_range);
  }

  public float getMax_range() {
    return playercore_javaJNI.get_player_laser_config_t_max_range(swigCPtr);
  }

  public void setRange_res(float range_res) {
    playercore_javaJNI.set_player_laser_config_t_range_res(swigCPtr, range_res);
  }

  public float getRange_res() {
    return playercore_javaJNI.get_player_laser_config_t_range_res(swigCPtr);
  }

  public void setIntensity(short intensity) {
    playercore_javaJNI.set_player_laser_config_t_intensity(swigCPtr, intensity);
  }

  public short getIntensity() {
    return playercore_javaJNI.get_player_laser_config_t_intensity(swigCPtr);
  }

  public player_laser_config_t() {
    this(playercore_javaJNI.new_player_laser_config_t(), true);
  }

}