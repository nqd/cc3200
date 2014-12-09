// Simplelink includes
#include "simplelink.h"

#include "network.h"


//*****************************************************************************
//
//! \brief Connecting to a WLAN Accesspoint using SmartConfig provisioning
//!
//! Enables SmartConfig provisioning for adding a new connection profile
//! to CC3200. Since we have set the connection policy to Auto, once
//! SmartConfig is complete, CC3200 will connect automatically to the new
//! connection profile added by smartConfig.
//!
//! \param[in]                     None
//!
//! \return                        None
//!
//! \note
//!
//! \warning                    If the WLAN connection fails or we don't
//!                             acquire an IP address, We will be stuck in this
//!                             function forever.
//
//*****************************************************************************
int SmartConfigConnect()
{
    unsigned char policyVal;
    long lRetVal = -1;

    // Clear all profiles 
    // This is of course not a must, it is used in this example to make sure
    // we will connect to the new profile added by SmartConfig
    //
    lRetVal = sl_WlanProfileDel(WLAN_DEL_ALL_PROFILES);
    ASSERT_ON_ERROR(lRetVal);

    //set AUTO policy
    lRetVal = sl_WlanPolicySet(  SL_POLICY_CONNECTION,
                      SL_CONNECTION_POLICY(1,0,0,0,1),
                      &policyVal,
                      1 /*PolicyValLen*/);
    ASSERT_ON_ERROR(lRetVal);

    // Start SmartConfig
    // This example uses the unsecured SmartConfig method
    //
    lRetVal = sl_WlanSmartConfigStart(0,                /*groupIdBitmask*/
                           SMART_CONFIG_CIPHER_NONE,    /*cipher*/
                           0,                           /*publicKeyLen*/
                           0,                           /*group1KeyLen*/
                           0,                           /*group2KeyLen */
                           NULL,                        /*publicKey */
                           NULL,                        /*group1Key */
                           NULL);                       /*group2Key*/
    ASSERT_ON_ERROR(lRetVal);

    // Wait for WLAN Event
    while((!IS_CONNECTED(g_ulStatus)) || (!IS_IP_ACQUIRED(g_ulStatus)))
    {
        _SlNonOsMainLoopTask();
    }
     //
     // Turn ON the RED LED to indicate connection success
     //
     GPIO_IF_LedOn(MCU_RED_LED_GPIO);
     //wait for few moments
     MAP_UtilsDelay(80000000);
     //reset to default AUTO policy
     lRetVal = sl_WlanPolicySet(  SL_POLICY_CONNECTION,
                           SL_CONNECTION_POLICY(1,0,0,0,0),
                           &policyVal,
                           1 /*PolicyValLen*/);
     ASSERT_ON_ERROR(lRetVal);

     return SUCCESS;
}
