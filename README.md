# 엘리베이터 제어 시스템 만들기

한 번쯤 엘리베이터를 기다리며 엘리베이터의 동작을 제어하는 시스템에 대해 
생각해 보셨을 것입니다.

본 문제는 엘리베이터를 제어하는 시스템을 구현하여 조금이라도 더 효율적으로 
승객을 운송하는 것이 목표입니다.

빌딩에 엘리베이터는 3대가 존재하며 최대 정원 수는 15명으로 고정되어 있으며
빌딩의 가장 낮은 층은 1층, 가장 높은 층은 30층입니다.

본 문제에서는 랜덤하게 만들어진 300명의 승객 요청이 주어지며
이 300명의 요청을 효율적으로 모두 처리하는 것이 목표입니다.


### 승객 요청
승객 요청은 id, 탑승층, 목적층에 대한 정보를 가지고 있고,

ElevatorSimulator 생성 시 인자로 전달한 callback 함수를 통해 얻을 수 있습니다.

승객 요청이 발생되는 시점은 아래와 같습니다.
1. ElevatorSimulator 가 생성될 때.
2. 밑에서 설명할 ElevatorSimulator::Tick 함수가 호출될 때.

```
// 승객 요청
struct Passenger
{
	int id;	// 승객 요청 id.
	int boardingFloor;	// 탑승층.
	int destinationFloor;	// 목적층.
};
std::queue<Passenger> passengers;

// Simulator 생성.
elev::ElevatorSimulator* elevSimulator = CreateElevatorSimulator(
	NUM_ELEVATORS, MAX_PASSENGERS_PER_ELEVATOR, MIN_FLOOR, MAX_FLOOR, NUM_PASSENTERS,
	[&passengers](int id, int boardingFloor, int destinationFloor)
{
	// 승객 요청이 발생하면 해당 callback이 호출된다.
	passengers.push(Passenger{ id, boardingFloor, destinationFloor });
});
```



### State, Event
엘리베이터는 5가지 이벤트(ElevatorEvent)를 통해 동작하게 되며 
각 이벤트에 따라 4가지 상태(ElevatorState)를 가지게 됩니다. 

```
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
```

ElevatorEvent 에 따른 ElevatorState 전환을 표로 나타내면 아래와 같습니다.

![fsmflowchart](https://drive.google.com/uc?id=1OVpT9g5YPowVin_YvfARVe82ICEY0zps)


### Order, Tick
엘리베이터에 ElevatorEvent 를 발생시키기 위해서는 ElevatorSimulator::Order 함수를 호출하여 각 엘리베이터의 행동을 정한 후

ElevatorSimulator::Tick 함수를 호출하여 시간을 경과시켜야 됩니다.

이렇게 Tick 함수가 호출되면 tick count 가 1 증가하고 각 엘리베이터는 ElevatorSimulator::Order 를 통해 미리 정의된 행동을 수행하게 됩니다. 

하나의 Tick 에서는 하나의 ElevatorEvent 가 수행되며 tick count 를 통해 시간이 얼마큼 경과되었는지를 알 수 있습니다.

```
elev::ElevatorSimulator* elevSimulator = CreateElevatorSimulator(
	NUM_ELEVATORS, MAX_PASSENGERS_PER_ELEVATOR, MIN_FLOOR, MAX_FLOOR, NUM_PASSENTERS,
	[&passengers](int id, int boardingFloor, int destinationFloor)
{
	// 승객 요청이 발생하면 해당 callback이 호출된다.
	passengers.push(Passenger{ id, boardingFloor, destinationFloor });
});

// 0번 엘리베이터 정지.
elevSimulator->Order(0, elev::ElevatorEvent::Stop);
// 1번 엘리베이터 5층까지 상승.
elevSimulator->Order(1, elev::ElevatorEvent::Up, 5);
// 2번 엘리베이터 문 열고 승객 1, 5, 7, 10 (passenger id) 엘리베이터에 승차.
std::vector<int> boardingPassengerIds{ 1, 5, 7, 10 };
elevSimulator->Order(2, elev::ElevatorEvent::OpenDoor, boardingPassengerIds);

elevSimulator->Tick();
```

### 엘리베이터 정보 얻기.
엘리베이터는 최초 생성 시 빌딩 최저층(1층)에 위치하고 정지해(ElevatorState::Stopped) 있습니다.

이후 변경되는 엘리베이터의 정보는 직접 작성하신 엘리베이터 제어 시스템에서 얻으실 수 있을 것입니다.

하지만 필요하실 경우 Simulator 에서도 두 가지 방법으로 엘리베이터 정보를 얻을 수 있습니다.

첫 번째 방법은 ElevatorSimulator 의 get 함수를 이용하는 방법입니다.
```
// 엘리베이터의 현재 위치(층) 반환.
int GetElevatorFloor(int elevatorIndex);
// 엘리베이터 현재 상태 반환.
ElevatorState GetElevatorState(int elevatorIndex);
// 엘리베이터에 현재 탑승한 승객 반환.
const std::vector<int>& GetElevatorPassengers(int elevatorIndex);
```

두 번째로 승객 요청을 얻는 것과 같은 방법으로 ElevatorSimulator 생성 시 전달한 callback 함수를 통해 얻을 수도 있습니다. 

Callback 함수는 엘리베이터가 생성되는 시점과 ElevatorSimulator::Tick 함수가 호출될 때 호출될 수 있습니다.
```
struct Passenger
{
	int id;	// 승객 요청 id.
	int boardingFloor;	// 탑승층.
	int destinationFloor;	// 목적층.
};

struct Elevator
{
	int floor;	// 현재 위치(층).
	elev::ElevatorState state;	// 현재 상태.
	std::vector<int> passsengers;	// 탑승 승객.
};

std::queue<Passenger> passengers;

Elevator elevator0 { MIN_FLOOR, elev::ElevatorState::Stopped, std::vector<int>() };

// Simulator 생성.
elev::ElevatorSimulator* elevSimulator = CreateElevatorSimulator(
	NUM_ELEVATORS, MAX_PASSENGERS_PER_ELEVATOR, MIN_FLOOR, MAX_FLOOR, NUM_PASSENTERS,
	[&passengers](int id, int boardingFloor, int destinationFloor)
{
	// 승객 요청이 발생하면 해당 callback이 호출된다.
	passengers.push(Passenger{ id, boardingFloor, destinationFloor });
},
	[&elevator0](int elevatorIndex, int floor, int state, const std::vector<int>& passengers)
{
	// 엘리베이터 상태가 변경되면 해당 callback이 호출된다.
	if (elevatorIndex == 0)
	{
		elevator0.floor = floor;
		elevator0.state = static_cast<elev::ElevatorState>(state);
		elevator0.passsengers = passengers;
	}
}
);
```


### 예제
아래 예제는 가장 무식한 방법으로 작성된 엘리베이터 제어 시스템입니다.

3대의 엘리베이터 중 한 대만 이용하고, 한 번에 한 명의 승객 요청만 처리하며

수행 중인 승객 요청이 완료될 때까지 다른 승객은 신경 쓰지 않습니다.

```
#include <iostream>
#include <queue>
#include "elevator/elevator.h"

#define NUM_ELEVATORS 3	// 빌딩의 엘리베이터 개수.
#define MAX_PASSENGERS_PER_ELEVATOR 15	// 하나의 엘리베이터 최대 정원.
#define MIN_FLOOR 1	// 빌딩 최저층.
#define MAX_FLOOR 30	// 빌딩 최고층.
#define NUM_PASSENTERS 300	// 승객 수

int main()
{
	struct Passenger
	{
		int id;	// 승객 요청 id.
		int boardingFloor;	// 탑승층.
		int destinationFloor;	// 목적층.
	};

	struct Elevator
	{
		int floor;	// 현재 위치(층).
		elev::ElevatorState state;	// 현재 상태.
		std::vector<int> passsengers;	// 탑승 승객.
	};

	std::queue<Passenger> passengers;

	// 본 예제에서는 무식하게 하나의 엘리베이터만 이용한다.
	Elevator elevator0 { MIN_FLOOR, elev::ElevatorState::Stopped, std::vector<int>() };

	// Simulator 생성.
	elev::ElevatorSimulator* elevSimulator = CreateElevatorSimulator(
		NUM_ELEVATORS, MAX_PASSENGERS_PER_ELEVATOR, MIN_FLOOR, MAX_FLOOR, NUM_PASSENTERS,
		[&passengers](int id, int boardingFloor, int destinationFloor)
	{
		// 승객 요청이 발생하면 해당 callback이 호출된다.
		passengers.push(Passenger{ id, boardingFloor, destinationFloor });
	},
		[&elevator0](int elevatorIndex, int floor, int state, const std::vector<int>& passengers)
	{
		// 엘리베이터 상태가 변경되면 해당 callback이 호출된다.
		if (elevatorIndex == 0)
		{
			elevator0.floor = floor;
			elevator0.state = static_cast<elev::ElevatorState>(state);
			elevator0.passsengers = passengers;
		}
	}
	);

	while (!elevSimulator->IsFinished())
	{
		if (passengers.size() == 0)
		{
			continue;
			elevSimulator->Tick();
		}
		const Passenger passenger = passengers.front();
		passengers.pop();

		// 승객이 탑승할 층으로 이동.
		while (elevator0.floor != passenger.boardingFloor)
		{
			elev::ElevatorEvent event = elevator0.floor < passenger.boardingFloor ?
				elev::ElevatorEvent::Up : elev::ElevatorEvent::Down;

			elevSimulator->Order(0, event, passenger.boardingFloor);
			elevSimulator->Tick();
		}

		// 정지시킨다.
		if (elevator0.state != elev::ElevatorState::Stopped)
		{
			elevSimulator->Order(0, elev::ElevatorEvent::Stop);
			elevSimulator->Tick();
		}

		// 문을 열고 승객 한 명만 태운다.
		{
			std::vector<int> boardingPassengerIds{ passenger.id };
			elevSimulator->Order(0, elev::ElevatorEvent::OpenDoor, boardingPassengerIds);
			if (elevSimulator->Tick())
			{
				// 완료.

				// // Simulator 삭제.
				DeleteElevatorSimulator(elevSimulator);
				return 0;
			}
		}

		// 문을 닫는다.
		{
			elevSimulator->Order(0, elev::ElevatorEvent::CloseDoor);
			elevSimulator->Tick();
		}

		// 목적층까지 이동.
		while (elevator0.floor != passenger.destinationFloor)
		{
			elev::ElevatorEvent event = elevator0.floor < passenger.destinationFloor ?
				elev::ElevatorEvent::Up : elev::ElevatorEvent::Down;

			elevSimulator->Order(0, event, passenger.destinationFloor);
			elevSimulator->Tick();
		}

		// 정지시킨다.
		{
			elevSimulator->Order(0, elev::ElevatorEvent::Stop);
			elevSimulator->Tick();
		}

		// 문을 열고 탑승한 승객을 내리게 한다.
		{
			elevSimulator->Order(0, elev::ElevatorEvent::OpenDoor);
			elevSimulator->Tick();
		}

		// 문을 닫는다.
		{
			elevSimulator->Order(0, elev::ElevatorEvent::CloseDoor);
			elevSimulator->Tick();
		}

		// 반복...
	}

	DeleteElevatorSimulator(elevSimulator);

	return 0;
}

```

### 문제 
이제 지원자께서는 객체 지향 원칙에 따라 300명의 승객 요청을 효율적으로 

모두 처리할 수 있는 엘리베이터 제어 시스템을 작성해 주시면 됩니다.

작성된 엘리베이터 제어 시스템은 위 예제와 같이 ElevatorSimulator 을 이용해 승객 요청을 모두 처리해 주셔야 됩니다.

필요한 객체를 자유롭게 만들어 모든 헤더 파일과 소스 파일을 제출해 주시면 검토 후 연락드리겠습니다. 

감사합니다.


### 설치
ElevatorSimulator는 본 문제 git 저장소의 lib/elevator/elevator.h, elevator.cpp 에 되어 있으며

권장하는 설치 방법은 본 문제 git 저장소를 clone 받은 후 Cmake 와 Visual studio 를 이용해 컴파일하시는 것을 추천드립니다.

컴파일을 위해서는 Visual studio [download][1]

와 Cmake 가 필요합니다 [download][2]

```
git clone https://github.com/JaykayChoi/motif_elev.git
cd motif_elev
mkdir _build
cd _build
cmake ..
```

완료되면 motif_elev.sin 을 Visual stuio 에서 열어주시면 됩니다.

[1]: https://www.visualstudio.com/downloads/
[2]: https://cmake.org/download/
