#ifndef FLOW_ONOFF_APPLICATION_H
#define FLOW_ONOFF_APPLICATION_H

#include "ns3/onoff-application.h"

namespace ns3 {

class FlowOnOffApplication : public OnOffApplication {
  public:
    static TypeId GetTypeId();

    FlowOnOffApplication();

    ~FlowOnOffApplication() override;

    void SendFlow();

  private:
    void StartApplication() override; // Called at time specified by Start
    void StopApplication() override;  // Called at time specified by Stop

    // helpers
    /**
     * \brief Cancel all pending events.
     */
    void CancelEvents() override;

    // Event handlers
    /**
     * \brief Start an On period
     */
    void StartSending() override;
    /**
     * \brief Start an Off period
     */
    void StopSending() override;

    /**
     * \brief Schedule the next On period start
     */
    void ScheduleStartEvent() override;
    /**
     * \brief Schedule the next Off period start
     */
    void ScheduleStopEvent() override;

    /**
     * \brief Handle a Connection Succeed event
     * \param socket the connected socket
     */
    void ConnectionSucceeded(Ptr<Socket> socket) override;
    /**
     * \brief Handle a Connection Failed event
     * \param socket the not connected socket
     */
    void ConnectionFailed(Ptr<Socket> socket) override;
};

} // namespace ns3

#endif