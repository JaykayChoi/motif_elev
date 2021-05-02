#include "elevator.h"
#include <cassert>

elev::ElevatorSimulator::ElevatorSimulator(
	int numElevators, 
	int maxPassengersPerElevator, 
	int minFloor, 
	int maxFloor,
	int numPassengers)
	: numElevators_(numElevators)
	, maxPassengersPerElevator_(maxPassengersPerElevator)
	, minFloor_(minFloor)
	, maxFloor_(maxFloor)
	, numPassengers_(numPassengers)
	, tickCount_(0)
{
	printf("Initializing elevator simulator...\n");

	BuildRandomPassengers();

	for (int i = 0; i < numElevators_; i++)
	{
		elevators_.push_back(Elevator(minFloor_));
		orders_.push_back(ElevatorOrder(minFloor_));
	}

	printf("Elevator simulator is initialized. "\
		"%d passengers are waiting... "\
		"Call Tick() after Order().\n"
		, numPassengers_);
}


elev::ElevatorSimulator::~ElevatorSimulator()
{
	delete[] totalPassengers_;
}

const elev::Passenger* elev::ElevatorSimulator::GetTotalPassengers()
{
	return totalPassengers_;
}

const std::map<int, elev::Passenger>& elev::ElevatorSimulator::GetRemainingPassengers()
{
	return remainingPassengers_;
}

const elev::Elevator* elev::ElevatorSimulator::GetElevator(int elevatorIndex)
{
	assert(elevatorIndex >= 0 && elevatorIndex <= numElevators_);
	return &elevators_[elevatorIndex];
}

ELEV_EXPORT void elev::ElevatorSimulator::Order(
	int elevatorIndex, 
	ElevatorEvent event, 
	int destinationFloor, 
	std::initializer_list<int> boardingPassengerIds)
{
	assert(elevatorIndex >= 0 && elevatorIndex <= numElevators_);

	Elevator elevator = elevators_[elevatorIndex];
	if (event == ElevatorEvent::Up)
	{
		assert(elevator.floor < destinationFloor);
	}
	else if (event == ElevatorEvent::Down)
	{
		assert(elevator.floor > destinationFloor);
	}

	ElevatorOrder& order = orders_[elevatorIndex];
	order.event = event;
	order.destinationFloor = destinationFloor;
	order.boardingPassengerIds = std::vector<int>(boardingPassengerIds);
}

ELEV_EXPORT void elev::ElevatorSimulator::Order(
	int elevatorIndex, 
	ElevatorEvent event, 
	std::initializer_list<int> boardingPassengerIds)
{
	assert(event != ElevatorEvent::Up && event != ElevatorEvent::Down);
	Order(elevatorIndex, event, 0, boardingPassengerIds);
}

bool elev::ElevatorSimulator::Tick()
{
	if (IsFinished())
	{
		printf("Finished already.");
		return true;
	}
	
	tickCount_++;

	for (int i = 0; i < numElevators_; i++)
	{
		ElevatorOrder& order = orders_[i];
		HandleEvent(i, order.event, order.boardingPassengerIds);

		// Tick 수행 결과 목표층에 도달한 경우 order event 를 Stop 으로 자동 변경해준다.
		// (추가 order 가 없더라도 엘리베이터를 정지시키기 위해)
		Elevator& elevator = elevators_[i];
		if ((order.event == ElevatorEvent::Up || order.event == ElevatorEvent::Down)
			&& elevator.floor == order.destinationFloor)
		{
			order.event = ElevatorEvent::Stop;
		}
	}

	int remaining = remainingPassengers_.size();
	if (remaining == 0)
	{
		printf("Finished. Tick count: %d\n", tickCount_);
		return true;
	}
	else
	{
		printf("Tick count: %d. Remaining passengers: %d. \n", 
			tickCount_, remaining);
	}

	return false;
}

bool elev::ElevatorSimulator::IsFinished()
{
	return remainingPassengers_.size() == 0;
}

ELEV_EXPORT void elev::ElevatorSimulator::Temp()
{
	remainingPassengers_.clear();
}

void elev::ElevatorSimulator::BuildRandomPassengers()
{
	totalPassengers_ = new Passenger[numPassengers_]();
	for (int i = 0; i < numPassengers_; i++)
	{
		Passenger p;
		p.id = i;
		int r = rand() % 100;
		if (r < 20)
		{
			p.boardingFloor = minFloor_;
			p.destinationFloor = RandInt(minFloor_ + 1, maxFloor_);
		}
		else if (r < 30)
		{
			p.boardingFloor = RandInt(minFloor_ + 1, maxFloor_);
			p.destinationFloor = minFloor_;
		}
		else
		{
			p.boardingFloor = RandInt(minFloor_, maxFloor_);
			p.destinationFloor = RandIntWithExclusion(
				minFloor_, maxFloor_, p.boardingFloor);
		}

		if (i > 50)
		{
			p.appearanceTickCount = RandInt(0, 100);
		}
		totalPassengers_[i] = p;
		remainingPassengers_.insert(std::pair<int, Passenger>(i, p));
	}
}

int elev::ElevatorSimulator::RandInt(int min, int max)
{
	assert(max > min);
	return rand() % (max - min + 1) + min;
}

int elev::ElevatorSimulator::RandIntWithExclusion(
	int min, int max, int exclude)
{
	int r = RandInt(min, max);
	if (r == exclude)
	{
		return RandIntWithExclusion(min, max, exclude);
	}
	else
	{
		return r;
	}
}

void elev::ElevatorSimulator::HandleEvent(
	int elevatorIndex, 
	ElevatorEvent event, 
	const std::vector<int>& passengerIds)
{
	assert(elevatorIndex >= 0 && elevatorIndex <= numElevators_);

	switch (event)
	{
	case ElevatorEvent::OpenDoor:
		HandleEventOpenDoor(elevatorIndex, passengerIds);
		break;
	case ElevatorEvent::CloseDoor:
		HandleEventCloseDoor(elevatorIndex);
		break;
	case ElevatorEvent::Up:
		HandleEventUp(elevatorIndex);
		break;
	case ElevatorEvent::Down:
		HandleEventDown(elevatorIndex);
		break;
	case ElevatorEvent::Stop:
		HandleEventStop(elevatorIndex);
		break;
	}
}

void elev::ElevatorSimulator::HandleEventOpenDoor(
	int elevatorIndex, const std::vector<int>& passengerIds)
{
	Elevator& elevator = elevators_[elevatorIndex];
	assert(elevator.state == ElevatorState::Stopped);

	elevator.state = ElevatorState::DoorIsOpened;

	// 승객 하차.
	std::vector<Passenger>::iterator it;
	for (it = elevator.passengers.begin(); it != elevator.passengers.end();)
	{
		if (it->destinationFloor == elevator.floor)
		{
			it = elevator.passengers.erase(it);
		}
		else
		{
			it++;
		}
	}

	// 승객 탑승
	for (int id : passengerIds)
	{
		std::map<int, Passenger>::iterator it = remainingPassengers_.find(id);
		assert(it != remainingPassengers_.end());

		assert(it->second.appearanceTickCount <= tickCount_);

		elevator.passengers.push_back(it->second);
		assert((int)elevator.passengers.size() <= maxPassengersPerElevator_);

		remainingPassengers_.erase(it);
	}
}

void elev::ElevatorSimulator::HandleEventCloseDoor(int elevatorIndex)
{
	Elevator& elevator = elevators_[elevatorIndex];
	assert(elevator.state == ElevatorState::DoorIsOpened);

	elevator.state = ElevatorState::Stopped;
}

void elev::ElevatorSimulator::HandleEventUp(int elevatorIndex)
{
	Elevator& elevator = elevators_[elevatorIndex];
	assert(elevator.state == ElevatorState::MovingUp ||
		elevator.state == ElevatorState::Stopped);

	elevator.floor++;
	assert(elevator.floor <= maxFloor_);

	elevator.state = ElevatorState::MovingUp;
}

void elev::ElevatorSimulator::HandleEventDown(int elevatorIndex)
{
	Elevator& elevator = elevators_[elevatorIndex];
	assert(elevator.state == ElevatorState::MovingDown ||
		elevator.state == ElevatorState::Stopped);

	elevator.floor--;
	assert(elevator.floor >= minFloor_);

	elevator.state = ElevatorState::MovingDown;
}

void elev::ElevatorSimulator::HandleEventStop(int elevatorIndex)
{
	Elevator& elevator = elevators_[elevatorIndex];
	assert(elevator.state == ElevatorState::MovingUp ||
		elevator.state == ElevatorState::MovingDown ||
		elevator.state == ElevatorState::Stopped);

	elevator.state = ElevatorState::Stopped;
}

// ----------------------------------------------------------------------------
// dll export
elev::ElevatorSimulator* CreateElevatorSimulator(
	int numElevators,
	int maxPassengersPerElevator,
	int minFloor,
	int maxFloor,
	int numPassengers)
{
	return new elev::ElevatorSimulator(
		numElevators, 
		maxPassengersPerElevator, 
		minFloor, 
		maxFloor, 
		numPassengers);
}

void DeleteElevatorSimulator(elev::ElevatorSimulator* simulator)
{
	delete simulator;
}
