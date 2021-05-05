#include "elevator.h"

elev::ElevatorSimulator::ElevatorSimulator(
	int numElevators, 
	int maxPassengersPerElevator, 
	int minFloor, 
	int maxFloor,
	int numPassengers,
	new_passenger_cb newPassengerCb)
	: numElevators_(numElevators)
	, maxPassengersPerElevator_(maxPassengersPerElevator)
	, minFloor_(minFloor)
	, maxFloor_(maxFloor)
	, numPassengers_(numPassengers)
	, tickCount_(0)
	, newPassengerCb_(newPassengerCb)
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

const elev::ElevatorSimulator::Elevator* elev::ElevatorSimulator::GetElevator(
	int elevatorIndex)
{
	ASSERT(elevatorIndex >= 0 && elevatorIndex < numElevators_, "Invalid elevator index");
	return &elevators_[elevatorIndex];
}

ELEV_EXPORT void elev::ElevatorSimulator::Order(
	int elevatorIndex, 
	ElevatorEvent event, 
	int destinationFloor, 
	std::vector<int>& boardingPassengerIds)
{
	ASSERT(elevatorIndex >= 0 && elevatorIndex < numElevators_, "Invalid elevator index");

	Elevator elevator = elevators_[elevatorIndex];
	if (event == ElevatorEvent::Up)
	{
		ASSERT(elevator.floor < destinationFloor, "Invalid destination floor");
	}
	else if (event == ElevatorEvent::Down)
	{
		ASSERT(elevator.floor > destinationFloor, "Invalid destination floor");
	}

	ElevatorOrder& order = orders_[elevatorIndex];
	order.event = event;
	order.destinationFloor = destinationFloor;
	order.boardingPassengerIds = boardingPassengerIds;
}

ELEV_EXPORT void elev::ElevatorSimulator::Order(
	int elevatorIndex, 
	ElevatorEvent event, 
	std::vector<int>& boardingPassengerIds)
{
	ASSERT(event != ElevatorEvent::Up && event != ElevatorEvent::Down, 
		"No destinaton floor");
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

	// 새로 발생한 승객 요청 callback
	for (auto& kv : remainingPassengers_)
	{
		auto p = kv.second;
		if (p.appearanceTickCount == tickCount_)
		{
			newPassengerCb_(p.id, p.boardingFloor, p.destinationFloor);
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

		if (p.appearanceTickCount == 0)
		{
			newPassengerCb_(p.id, p.boardingFloor, p.destinationFloor);
		}
	}
}

int elev::ElevatorSimulator::RandInt(int min, int max)
{
	ASSERT(max > min, "Max is greater than min");
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
	ASSERT(elevatorIndex >= 0 && elevatorIndex < numElevators_, "Invalid elevator index");

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
	ASSERT(elevator.state == ElevatorState::Stopped, "Cannot open door unless stopped");

	elevator.state = ElevatorState::DoorIsOpened;

	// 승객 하차.
	std::vector<int>::iterator it;
	for (it = elevator.passengerIds.begin(); it != elevator.passengerIds.end();)
	{
		int passengerId = *it;
		ASSERT(passengerId >= 0 && passengerId < numPassengers_, "Invalid passenger id");
		Passenger p = totalPassengers_[passengerId];		
		
		if (p.destinationFloor == elevator.floor)
		{
			it = elevator.passengerIds.erase(it);
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
		ASSERT(it != remainingPassengers_.end(), "No passenger");

		ASSERT(it->second.appearanceTickCount <= tickCount_, "No passenger");

		elevator.passengerIds.push_back(it->second.id);
		ASSERT((int)elevator.passengerIds.size() <= maxPassengersPerElevator_,
			"Exceeds elevator capacity");

		remainingPassengers_.erase(it);
	}
}

void elev::ElevatorSimulator::HandleEventCloseDoor(int elevatorIndex)
{
	Elevator& elevator = elevators_[elevatorIndex];
	ASSERT(elevator.state == ElevatorState::DoorIsOpened, "Cannot close door");

	elevator.state = ElevatorState::Stopped;
}

void elev::ElevatorSimulator::HandleEventUp(int elevatorIndex)
{
	Elevator& elevator = elevators_[elevatorIndex];
	ASSERT(elevator.state == ElevatorState::MovingUp ||
		elevator.state == ElevatorState::Stopped, "Cannot up");

	elevator.floor++;
	ASSERT(elevator.floor <= maxFloor_, "Cannot up any more");

	elevator.state = ElevatorState::MovingUp;
}

void elev::ElevatorSimulator::HandleEventDown(int elevatorIndex)
{
	Elevator& elevator = elevators_[elevatorIndex];
	ASSERT(elevator.state == ElevatorState::MovingDown ||
		elevator.state == ElevatorState::Stopped, "Cannot down");

	elevator.floor--;
	ASSERT(elevator.floor >= minFloor_, "Cannot down any more");

	elevator.state = ElevatorState::MovingDown;
}

void elev::ElevatorSimulator::HandleEventStop(int elevatorIndex)
{
	Elevator& elevator = elevators_[elevatorIndex];
	ASSERT(elevator.state == ElevatorState::MovingUp ||
		elevator.state == ElevatorState::MovingDown ||
		elevator.state == ElevatorState::Stopped, "Cannot stop");

	elevator.state = ElevatorState::Stopped;
}

// ----------------------------------------------------------------------------
// dll export
elev::ElevatorSimulator* CreateElevatorSimulator(
	int numElevators,
	int maxPassengersPerElevator,
	int minFloor,
	int maxFloor,
	int numPassengers,
	new_passenger_cb newPassengerCb)
{
	return new elev::ElevatorSimulator(
		numElevators, 
		maxPassengersPerElevator, 
		minFloor, 
		maxFloor, 
		numPassengers,
		newPassengerCb);
}

void DeleteElevatorSimulator(elev::ElevatorSimulator* simulator)
{
	delete simulator;
}
