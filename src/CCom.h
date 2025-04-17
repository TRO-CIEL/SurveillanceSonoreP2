/**
 * @file CCom.h
 * @brief Communication class header
 */

#ifndef CCOM_H
#define CCOM_H

#include <WiFi.h>
#include <HTTPClient.h>

/**
 * @brief Class for communication over WiFi
 */
class CCom
{
public:
  /**
   * @brief Constructor for CCom class
   */
  CCom();

  /**
   * @brief Setup WiFi connection
   * @return true if successful
   */
  bool Setup();

  /**
   * @brief Send data via HTTP
   * @param data Data to send
   * @return true if successful
   */
  bool SendData(String data);

  // Public attributes
  String data; ///< Data to be sent
};

#endif // CCOM_H