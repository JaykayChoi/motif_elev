#ifndef LIB_ELEVATOR_H_INCLUDED
#define LIB_ELEVATOR_H_INCLUDED

#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include <cstdarg>

#define ELEV_EXPORT __declspec(dllexport)
#define ELEV_IMPORT __declspec(dllimport)

#define ASSERT(condition, msg) \
    do { \
        if (! (condition)) { \
            std::cerr << msg << ": assertion `" #condition "` failed in " << __FILE__ \
                      << " line " << __LINE__ << std::endl; \
            std::terminate(); \
        } \
    } while (false)

typedef std::function<void(
	int /* id */,
	int /* boardingFloor */,
	int /* destinationFloor */)> new_passenger_cb;

typedef std::function<void(
	int /* elevatorIndex */,
	int /* floor */,
	int /* state */,
	const std::vector<int>& /* passengerIds */)> elevator_transition_cb;

namespace elev {

enum class ElevatorState
{
	// 엘리베이터 문이 열린 상태. 
	// 문이 열림과 동시에 승객이 타고 내린다고 가정한다.
	DoorIsOpened,

	// 상승 중.
	MovingUp,

	// 하강 중.
	MovingDown,

	// 정지한 상태.
	Stopped,
};

enum class ElevatorEvent
{
	OpenDoor,

	CloseDoor,

	Up,

	Down,

	Stop,
};

class ElevatorSimulator
{
public:

	// @parma numElevators 빌딩의 엘리베이터 개수.
	// @parma maxPassengersPerElevator 엘리베이터 하나의 최대 승객 수.
	// @parma minFloor 빌딩 최저층.
	// @parma maxFloor 빌딩 최고층.
	// @parma numPassengers 시뮬레이터 총 승객 수.
	// @parma newPassengerCb 새로운 승객 요청이 발생할 경우 호출되는 callback 함수.
	// @parma elevatorTransitionCb 엘리베이터 상태가 변경될 경우 호출되는 callback 함수.
	ElevatorSimulator(
		int numElevators, 
		int maxPassengersPerElevator, 
		int minFloor, 
		int maxFloor,
		int numPassengers,
		new_passenger_cb newPassengerCb,
		elevator_transition_cb elevatorTransitionCb);

	~ElevatorSimulator();

	// 엘리베이터의 현재 위치(층) 반환.
	ELEV_EXPORT int GetElevatorFloor(int elevatorIndex);
	// 엘리베이터 현재 상태 반환.
	ELEV_EXPORT ElevatorState GetElevatorState(int elevatorIndex);
	// 엘리베이터에 현재 탑승한 승객 반환.
	ELEV_EXPORT const std::vector<int>& GetElevatorPassengers(int elevatorIndex);

	// TODO rename schedule? reserve?
	// 엘리베이터에 명령을 내린다. 접수된 명령은 다음 tick 에 수행된다.
	// 엘리베이터에 이미 접수된 명령이 있을 경우 덮어써진다.
	// @param elevatorIndex elevator index. 
	// @param event elevator 가 수행할 ElevatorEvent
	// @param destinationFloor 목적층. event 가 Up 또는 Down 일 때만 유효하다.
	// @param boardingPassengerIds 탑승하는 passenger id. event 가 OpenDoor 일 때만 유효하다.
	ELEV_EXPORT void Order(
		int elevatorIndex,
		ElevatorEvent event,
		int destinationFloor = 0,
		std::vector<int>& boardingPassengerIds = std::vector<int>());
	ELEV_EXPORT void Order(
		int elevatorIndex,
		ElevatorEvent event,
		std::vector<int>& boardingPassengerIds);

	// Simulator를 one tick 진행시킨다.
	// 승객 요청을 모두 처리한 경우 true 반환.
	ELEV_EXPORT bool Tick();

	// 현재 tick count 반환.
	ELEV_EXPORT int TickCount() { return tickCount_; }

	// 승객 요청이 모두 처리되었을 경우 true 반환.
	ELEV_EXPORT bool IsFinished();

private:

	struct Passenger
	{
		int id;	
		int boardingFloor;	
		int destinationFloor;	
		int appearanceTickCount;

		Passenger() 
			: id(0)
			, boardingFloor(0)
			, destinationFloor(0)
			, appearanceTickCount(0)
		{};
	};

	struct Elevator
	{
		int floor;	
		std::vector<int> passengerIds;	
		ElevatorState state;	

		Elevator(int defaultFloor)
			: floor(defaultFloor)
			, state(ElevatorState::Stopped)
		{};
	};

	int numElevators_;
	int maxPassengersPerElevator_;
	int minFloor_;
	int maxFloor_;
	int numPassengers_;

	int tickCount_;

	std::vector<Elevator> elevators_;

	Passenger* totalPassengers_;
	std::map<int, Passenger> remainingPassengers_;

	new_passenger_cb newPassengerCb_;
	elevator_transition_cb elevatorTransitionCb_;

	struct ElevatorOrder {
		ElevatorEvent event;
		int destinationFloor;
		std::vector<int> boardingPassengerIds;

		ElevatorOrder(int defaultFloor)
			: event(ElevatorEvent::Stop)
			, destinationFloor(defaultFloor)
			, boardingPassengerIds(std::vector<int>())
		{};
	};
	std::vector<ElevatorOrder> orders_;
	
	void BuildRandomPassengers();

	int RandInt(int min, int max);

	int RandIntWithExclusion(int min, int max, int exclude);

	void HandleEvent(
		int elevatorIndex, 
		ElevatorEvent event, 
		const std::vector<int>& passengerIds);

	void HandleEventOpenDoor(
		int elevatorIndex, const std::vector<int>& passengerIds);
	void HandleEventCloseDoor(int elevatorIndex);
	void HandleEventUp(int elevatorIndex);
	void HandleEventDown(int elevatorIndex);
	void HandleEventStop(int elevatorIndex);

};
}	// namespace elev

// ----------------------------------------------------------------------------
// dll export

// @parma numElevators 빌딩의 엘리베이터 개수.
// @parma maxPassengersPerElevator 엘리베이터 하나의 최대 승객 수.
// @parma minFloor 빌딩 최저층.
// @parma maxFloor 빌딩 최고층.
// @parma numPassengers 시뮬레이터 총 승객 수.
// @parma newPassengerCb 새로운 승객 요청이 발생할 경우 호출되는 callback 함수.
// @parma elevatorTransitionCb 엘리베이터 상태가 변경될 경우 호출되는 callback 함수.
ELEV_EXPORT elev::ElevatorSimulator* CreateElevatorSimulator(
	int numElevators,
	int maxPassengersPerElevator,
	int minFloor,
	int maxFloor,
	int numPassengers,
	new_passenger_cb passengerCallCallback,
	elevator_transition_cb elevatorTransitionCb = nullptr);

ELEV_EXPORT void DeleteElevatorSimulator(elev::ElevatorSimulator* simulator);

#endif // LIB_ELEVATOR_H_INCLUDED
