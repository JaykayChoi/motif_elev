﻿#ifndef LIB_ELEVATOR_H_INCLUDED
#define LIB_ELEVATOR_H_INCLUDED

#define ELEV_EXPORT __declspec(dllexport)
#define ELEV_IMPORT __declspec(dllimport)


#include <iostream>
#include <vector>
#include <map>

namespace elev {

// 승객 요청
struct Passenger
{
	int id;	// 승객 id.
	int boardingFloor;	// 탑층층.
	int destinationFloor;	// 목적층.

	// 등장 시간. 
	// Simulator의 tickCount 가 appearanceTickCount 보다 큰 경우에만 
	// 해당 승객 요청이 유효하다.
	int appearanceTickCount;	

	Passenger()
		: id(0)
		, boardingFloor(0)
		, destinationFloor(0)
		, appearanceTickCount(0)
	{};
};

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

struct Elevator
{
	int floor;	// 현재 위치한 층.
	std::vector<Passenger> passengers;	// 탑승하고 있는 승객들.
	ElevatorState state;	// 엘리베이터 상태.

	Elevator(int defaultFloor)
		: floor(defaultFloor)
		, state(ElevatorState::Stopped)
	{};
};

class ElevatorSimulator
{
public:
	
	ElevatorSimulator(
		int numElevators, 
		int maxPassengersPerElevator, 
		int minFloor, 
		int maxFloor,
		int numPassengers);

	~ElevatorSimulator();

	// 전체 승객 반환.
	ELEV_EXPORT const Passenger* GetTotalPassengers();

	// 처리되지 않은 승객 반환.
	ELEV_EXPORT const std::map<int, Passenger>& GetRemainingPassengers();

	// Elevator 반환.
	ELEV_EXPORT const Elevator* GetElevator(int elevatorIndex);

	// TODO rename
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
		std::initializer_list<int> boardingPassengerIds = {});
	ELEV_EXPORT void Order(
		int elevatorIndex,
		ElevatorEvent event,
		std::initializer_list<int> boardingPassengerIds);

	// Simulator를 one tick 진행시킨다.
	// 승객 요청을 모두 처리한 경우 true 반환.
	ELEV_EXPORT bool Tick();

	// 현재 tick count 반환.
	ELEV_EXPORT int TickCount() { return tickCount_; }

	ELEV_EXPORT bool IsFinished();

	ELEV_EXPORT void Temp();

private:
	int numElevators_;
	int maxPassengersPerElevator_;
	int minFloor_;
	int maxFloor_;
	int numPassengers_;

	int tickCount_;

	std::vector<Elevator> elevators_;

	Passenger* totalPassengers_;
	std::map<int, Passenger> remainingPassengers_;

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
ELEV_EXPORT elev::ElevatorSimulator* CreateElevatorSimulator(
	int numElevators,
	int maxPassengersPerElevator,
	int minFloor,
	int maxFloor,
	int numPassengers);

ELEV_EXPORT void DeleteElevatorSimulator(elev::ElevatorSimulator* simulator);

#endif // LIB_ELEVATOR_H_INCLUDED
