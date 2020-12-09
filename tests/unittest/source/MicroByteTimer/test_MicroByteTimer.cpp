#include "gtest/gtest.h"

#include "MicroByteRTOS.h"
#include "MicroByteUnitTest.h"

#include "MicroBytePeriphTimer.h"
#include "periphTimer.h"

using namespace microbyte;

enum
{
    callCountIndexSet = 0,
    callCountIndexCancel,
    callCountIndexTimerHandler,
    callCountIndexMax
};

static uint32_t testTimerNow;
static uint32_t testCallCount[callCountIndexMax];

void testPeriphTimerSet(TimerClock *clock, uint32_t value);
uint32_t testPeriphTimerNow(TimerClock *clock);
uint32_t testPeriphTimerNow(TimerClock *clock);
void testPeriphTimerCancel(TimerClock *clock);
void testPeriphTimerIsrCallback(void *arg, int channel);


class TestMicroByteTimer : public testing::Test
{
    protected:
    MicroByteCpuTest *cpuTest;
    TimerOperations *timerOperations;
    PeriphTimer *periphTimerUsec;
    TimerClock *TIMER_USEC;

    void initTestTimers()
    {
        gtestPeriphTimerSet = testPeriphTimerSet;
        gtestPeriphTimerNow = testPeriphTimerNow;
        gtestPeriphTimerCancel = testPeriphTimerCancel;
        gtestPeriphTimerIsrCallback = testPeriphTimerIsrCallback;
    }

    void initCounters()
    {
        testTimerNow = 0;
        memset(testCallCount, 0, sizeof(testCallCount));
    }

    virtual void SetUp()
    {
        cpuTest = new MicroByteCpuTest();
        microbyte::cpuSet(cpuTest);

        timerOperations = new TimerOperations();
        timerOperations->set = periphTimerSet;
        timerOperations->now = periphTimerNow;
        timerOperations->cancel = periphTimerCancel;

        periphTimerUsec = new PeriphTimer(timerOperations,
                                          MICROBYTE_CONFIG_TIMER_USEC_DEV,
                                          MICROBYTE_CONFIG_TIMER_USEC_MIN,
                                          MICROBYTE_CONFIG_TIMER_USEC_BASE_FREQ);

        TIMER_USEC = periphTimerUsec->clock();

        initTestTimers();
        initCounters();
    }

    virtual void TearDown()
    {
        delete cpuTest;
        delete timerOperations;
        delete periphTimerUsec;
    }
};

TEST_F(TestMicroByteTimer, constructorTest)
{
    EXPECT_TRUE(timerOperations);
    EXPECT_TRUE(periphTimerUsec);
}

class TestTimer : public Timer
{
    uint32_t firedCounter;

    public:

    explicit TestTimer(TimerClock *clock)
        : Timer(clock, TestTimer::handleTimerFired, this)
        , firedCounter(0)
    {
    }

    static void handleTimerFired(void *arg)
    {
        reinterpret_cast<TestTimer *>(arg)->handleTimerFired();
    }

    void handleTimerFired()
    {
        testCallCount[callCountIndexTimerHandler]++;
        firedCounter++;
    }

    uint32_t getFiredCounter() { return firedCounter; }

    void resetFiredCounter() { firedCounter = 0; }
};

TEST_F(TestMicroByteTimer, oneTimerTest)
{
    const uint32_t timeT0 = 1000;
    const uint32_t timerInterval = 10;
    TestTimer timer = TestTimer(TIMER_USEC);

    initCounters();

    EXPECT_EQ(TIMER_USEC->now(), 0);

    testTimerNow = timeT0;

    EXPECT_EQ(TIMER_USEC->now(), timeT0);

    timer.set(timerInterval);

    EXPECT_EQ(testCallCount[callCountIndexTimerHandler], 0);
    EXPECT_EQ(timer.getFiredCounter(), 0);

    testTimerNow += timerInterval;

    periphTimerIsrCallback(TIMER_USEC, 0);

    EXPECT_EQ(testCallCount[callCountIndexTimerHandler], 1);
    EXPECT_EQ(timer.getFiredCounter(), 1);

    periphTimerIsrCallback(TIMER_USEC, 0);
    periphTimerIsrCallback(TIMER_USEC, 0);
    periphTimerIsrCallback(TIMER_USEC, 0);
    periphTimerIsrCallback(TIMER_USEC, 0);

    // There is no timer on the TimerClock list, so nothing will fired

    EXPECT_EQ(testCallCount[callCountIndexTimerHandler], 1);
    EXPECT_EQ(timer.getFiredCounter(), 1);
}

TEST_F(TestMicroByteTimer, twoTimerTest)
{
    const uint32_t timeT0 = 1000;
    const uint32_t timerInterval = 10;

    TestTimer timer1 = TestTimer(TIMER_USEC);
    TestTimer timer2 = TestTimer(TIMER_USEC);

    initCounters();

    EXPECT_EQ(TIMER_USEC->now(), 0);

    testTimerNow = timeT0;

    timer1.set(timerInterval);

    EXPECT_EQ(testCallCount[callCountIndexTimerHandler], 0);
    EXPECT_EQ(timer1.getFiredCounter(), 0);
    EXPECT_EQ(timer2.getFiredCounter(), 0);

    testTimerNow += timerInterval;

    timer2.set(timerInterval);

    periphTimerIsrCallback(TIMER_USEC, 0);

    EXPECT_EQ(testCallCount[callCountIndexTimerHandler], 1);
    EXPECT_EQ(timer1.getFiredCounter(), 1);
    EXPECT_EQ(timer2.getFiredCounter(), 0);

    testTimerNow += timerInterval;

    periphTimerIsrCallback(TIMER_USEC, 0);

    EXPECT_EQ(testCallCount[callCountIndexTimerHandler], 2);
    EXPECT_EQ(timer1.getFiredCounter(), 1);
    EXPECT_EQ(timer2.getFiredCounter(), 1);
}

void testPeriphTimerSet(TimerClock *clock, uint32_t value)
{
    (void) value;
    testCallCount[callCountIndexSet]++;
}

uint32_t testPeriphTimerNow(TimerClock *clock)
{
    (void)clock;
    return testTimerNow;
}

void testPeriphTimerCancel(TimerClock *clock)
{
    (void)clock;
    testCallCount[callCountIndexCancel]++;
}

void testPeriphTimerIsrCallback(void *arg, int channel)
{
    (void)channel;
    TimerClock *clock = reinterpret_cast<TimerClock *>(arg);
    clock->handler();
}
