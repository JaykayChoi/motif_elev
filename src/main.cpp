#include <iostream>
#include "elevator/elevator.h"

#define NUM_ELEVATORS 3	// 빌딩의 엘리베이터 개수.
#define MAX_PASSENGERS_PER_ELEVATOR 15	// 하나의 엘리베이터 최대 정원.
#define MIN_FLOOR 1	// 빌딩 최저층.
#define MAX_FLOOR 30	// 빌딩 최고층.
#define NUM_PASSENTERS 300	// 승객 수

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_CLIENTBLOCK   new(_CLIENT_BLOCK, __FILE__, __LINE__)
#define new DEBUG_CLIENTBLOCK
#endif

int main()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

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

