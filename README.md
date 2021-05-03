# 엘리베이터 제어 시스템 만들기

한 번쯤 엘리베이터를 기다리며 엘리베이터의 동작을 제어하는 시스템에 대해 
생각 해보셨을 것입니다.

본 문제는 엘리베이터를 제어하는 시스템을 구현하여 조금이라도 더 효율적으로 
승객을 운송하는 것이 목표입니다.

빌딩에 엘리베이터는 3대가 존재하며 최대 정원 수는 15명으로 고정되어 있으며
빌딩의 가장 낮은 층은 1층, 가장 높은 층은 30층입니다.

본 문제에서는 랜덤하게 만들어진 300명의 승객 요청이 주어지며
이 300명의 요청을 효율적으로 모두 처리하는 것이 목표입니다.


### 승객 요청
승객 요청은 id, 탑승층, 목적층 그리고 등장 시간 정보를 가지고 있습니다.
```
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
};
```

등장 시간은 tick count 라는 단위를 사용하는데 tick count 는 본 문제에서 시간의 경과를 표현하기 위해 사용되는 개념입니다.

tick count 는 뒤에서 설명될 ElevatorSimulator::Tick 함수가 호출될 때 마다 1씩 증가합니다.

만약 승객 요청의 등장 시간이 10이라면 ElevatorSimulator 의 tickCount 값이
10 이상일 때 해당 승객의 요청이 유효하다는 의미입니다.



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


# Order, Tick
엘리베이터에 ElevatorEvent 를 발생시키기 위해서는 ElevatorSimulator::Order 함수를 호출하여 각 엘리베이터의 행동을 정한 후

ElevatorSimulator::Tick 함수를 호출하여 시간을 경과시켜야 됩니다.

```
elev::ElevatorSimulator* elevSimulator = CreateElevatorSimulator(
	NUM_ELEVATORS, MAX_PASSENGERS_PER_ELEVATOR, MIN_FLOOR, MAX_FLOOR, NUM_PASSENTERS);
elevSimulator->Order(0, elev::ElevatorEvent::Stop);
elevSimulator->Tick();
```

ElevatorSimulator::Tick 함수가 호출되면 tickCount 가 1 증가하고 

각 엘리베이터는 ElevatorSimulator::Order 를 통해 미리 정의된 행동을 수행하게 됩니다. 

하나의 Tick 에서는 하나의 ElevatorEvent 가 수행됩니다.

### 예제
아래 예제는 가장 무식한 방법으로 구현된 엘리베이터 제어 시스템입니다.

3대의 엘리베이터 중 한 대만 이용하고, 한 번에 한 명의 승객 요청만 다루며

수행 중인 승객 요청이 완료될 때 까지 다른 승객은 신경쓰지 않습니다.
```
#include <iostream>
#include "elevator/elevator.h"

#define NUM_ELEVATORS 3	// 빌딩의 엘리베이터 개수.
#define MAX_PASSENGERS_PER_ELEVATOR 15	// 하나의 엘리베이터 최대 정원.
#define MIN_FLOOR 1	// 빌딩 최저층.
#define MAX_FLOOR 30	// 빌딩 최고층.
#define NUM_PASSENTERS 300	// 승객 수

int main() 
{
	// Simulator 생성.
	elev::ElevatorSimulator* elevSimulator = CreateElevatorSimulator(
		NUM_ELEVATORS, MAX_PASSENGERS_PER_ELEVATOR, MIN_FLOOR, MAX_FLOOR, NUM_PASSENTERS);

	const elev::Passenger* passengers = elevSimulator->GetTotalPassengers();
	// 본 예제에서는 무식하게 하나의 엘리베이터만 이용한다.
	const elev::Elevator* elevator0 = elevSimulator->GetElevator(0);
	for (int i = 0; i < NUM_PASSENTERS; i++)
	{
		const elev::Passenger passenger = passengers[i];

		// 승객 요청이 발생할(유효해질) 때 까지 기다린다.
		while (passenger.appearanceTickCount > elevSimulator->TickCount() + 1)
		{
			elevSimulator->Tick();
		}

		// 승객이 탑승할 층으로 이동.
		while (elevator0->floor != passenger.boardingFloor)
		{
			elev::ElevatorEvent event = elevator0->floor < passenger.boardingFloor ?
				elev::ElevatorEvent::Up : elev::ElevatorEvent::Down;

			elevSimulator->Order(0, event, passenger.boardingFloor);
			elevSimulator->Tick();
		}

		// 정지시킨다.
		if (elevator0->state != elev::ElevatorState::Stopped)
		{
			elevSimulator->Order(0, elev::ElevatorEvent::Stop);
			elevSimulator->Tick();
		}

		// 문을 열고 승객 한 명만 태운다.
		{
			elevSimulator->Order(0, elev::ElevatorEvent::OpenDoor, { passenger.id });
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
		while (elevator0->floor != passenger.destinationFloor)
		{
			elev::ElevatorEvent event = elevator0->floor < passenger.destinationFloor ?
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

모두 처리할 수 있는 엘리베이터 제어 시스템을 설계해 주시면 됩니다.

설계된 엘리베이터 제어 시스템은 위 예제와 같이 ElevatorSimulator 을 이용해 승객 요청을 모두 처리해주셔야 됩니다.

필요한 객체를 자유롭게 만들어 모든 헤더 파일과 소스 파일을 제출해주시면 검토 후 연락드리겠습니다. 

감사합니다.


### 설치
ElevatorSimulator는 본 문제 git 저장소의 lib/elevator/elevator.h, elevator.cpp 에 구현되어 있으며

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
