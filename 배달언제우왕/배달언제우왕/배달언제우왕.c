#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// 가능한 구역과 메뉴에 따른 배달 소요 시간 (분 단위)
const char* availableAreas[] = { "복대1동", "복대2동", "가경동", "성화개신죽림동", "봉명1동", "사창동", "사직2동", "수곡2동" };
const int addressDeliveryTimes[] = { 11, 4, 13, 6, 7, 4, 8, 6 };
const char* menuNames[] = { "후라이드치킨", "양념치킨", "간장치킨", "어니언치킨", "자장면", "짬뽕", "군만두", "탕수육",
                            "족발", "불족발", "보쌈", "막국수", "가츠동", "제육덮밥", "회덮밥", "사케동", "냉면", "비빔냉면",
                            "육전냉면", "육전", "수육국밥", "순대국밥", "내장국밥", "섞어국밥" };
const int menuDeliveryTimes[] = { 8, 10, 12, 11, 9, 15, 7, 20, 12, 14, 20, 8, 15, 10, 8, 8, 8, 9, 13, 5, 13, 5, 7, 15 };

// 가게별 영업 시간
const char* storeOpeningHours[] = { "18:00-24:00", "10:00-20:00", "12:00-21:00", "17:00-05:00", "00:00-24:00", "09:00-19:00" };

// 주문 상태를 나타내는 열거형
typedef enum {
    COOKING,     // 조리 중
    DELIVERING,  // 배달 중
    DELIVERED    // 배달 완료
} OrderStatus;

// 주문 상태를 문자열로 변환하는 함수
const char* getOrderStatusString(OrderStatus status) {
    switch (status) {
    case COOKING:
        return "조리 중";
    case DELIVERING:
        return "배달 중";
    case DELIVERED:
        return "배달 완료";
    default:
        return "알 수 없음";
    }
}

// 주문 내역을 저장하는 구조체
typedef struct {
    char time[30];    // 주문 시간
    char name[100];   // 이름
    char address[100]; // 주소
    char menu[100];   // 메뉴
    int count;        // 수량
    OrderStatus status; // 주문 상태
    int arrivalTime;   // 예상 도착 시간 (분 단위)
} Order;

// 주문 내역을 저장하는 배열
Order* orders = NULL;
int numOrders = 0; // 저장된 주문 수

void printMenu() {
    printf("================================================\n\n");
    printf("                 1. 주문하기\n\n");
    printf("                 2. 남은시간 조회\n\n");
    printf("                 3. 종료하기\n\n");
    printf("================================================\n");
    printf("메뉴를 선택해주세요: ");
}

void printStoreMenu(const char* filename) {
    FILE* file;
    char line[100]; // 줄을 읽어올 버퍼

    // 파일을 열고 읽기 모드로 설정합니다.
    file = fopen(filename, "r");
    if (file == NULL) {
        printf("파일을 열 수 없습니다.\n");
        return;
    }

    // 파일 내용 읽기 및 출력
    while (fgets(line, sizeof(line), file) != NULL) {
        // 개행 문자 제거 후 출력
        printf("%s", line);
    }

    // 파일 닫기
    fclose(file);
}

// 사용자가 입력한 주소가 가능한 구역인지 확인하는 함수
bool isAddressValid(const char* address) {
    for (int i = 0; i < sizeof(availableAreas) / sizeof(availableAreas[0]); ++i) {
        if (strcmp(address, availableAreas[i]) == 0) {
            return true; // 주소가 가능한 구역 중 하나와 일치하는 경우 true 반환
        }
    }
    return false; // 주소가 가능한 구역 중 어느 곳과도 일치하지 않는 경우 false 반환
}

// 주문 리스트에서 이름과 주소가 일치하는 주문을 찾는 함수
Order* findOrder(const char* name, const char* address) {
    for (int i = 0; i < numOrders; ++i) {
        if (strcmp(orders[i].name, name) == 0 && strcmp(orders[i].address, address) == 0) {
            return &orders[i]; // 일치하는 주문을 찾으면 해당 주문의 포인터 반환
        }
    }
    return NULL; // 일치하는 주문이 없을 경우 NULL 반환
}

// 예상 도착 시간 계산 함수
int calculateDeliveryTime(const char* address, const char* menu) {
    int addressTime = 0;
    int menuTime = 0;

    // 주소에 따른 배달 시간 계산
    for (int i = 0; i < sizeof(availableAreas) / sizeof(availableAreas[0]); ++i) {
        if (strcmp(address, availableAreas[i]) == 0) {
            addressTime = addressDeliveryTimes[i];
            break;
        }
    }

    // 메뉴에 따른 배달 시간 계산
    for (int i = 0; i < sizeof(menuNames) / sizeof(menuNames[0]); ++i) {
        if (strcmp(menu, menuNames[i]) == 0) {
            menuTime = menuDeliveryTimes[i];
            break;
        }
    }

    return addressTime + menuTime;
}

// 예상 도착 시간 계산 함수
void calculateArrivalTime(const char* timeStr, int deliveryTime, int* arrivalHour, int* arrivalMin) {
    int currentHour, currentMin;
    sscanf(timeStr, "%d:%d", &currentHour, &currentMin); // 현재 시간을 시와 분으로 분리

    // 현재 시간을 분 단위로 계산
    int currentTime = currentHour * 60 + currentMin;

    // 예상 도착 시간을 현재 시간에 배달 소요 시간을 더하여 계산
    int totalMinutes = currentTime + deliveryTime;

    // 현재 시간이 24시를 넘어갈 경우 24시간으로 나눈 나머지 계산
    totalMinutes %= (24 * 60);

    // 예상 도착 시간의 시와 분 계산
    *arrivalHour = totalMinutes / 60;
    *arrivalMin = totalMinutes % 60;
}

// 주문 내역을 배열에 추가하는 함수
void addOrder(const char* name, const char* address, const char* menu, int count, const char* time, int arrivalTime) {
    // 새로운 주문 내역을 저장할 공간 할당
    orders = realloc(orders, (numOrders + 1) * sizeof(Order));
    if (orders == NULL) {
        printf("메모리를 할당할 수 없습니다.\n");
        return;
    }

    // 주문 내역 추가
    strcpy(orders[numOrders].time, time);
    strcpy(orders[numOrders].name, name);
    strcpy(orders[numOrders].address, address);
    strcpy(orders[numOrders].menu, menu);
    orders[numOrders].count = count;
    orders[numOrders].status = COOKING; // 새 주문은 조리 중 상태로 초기화
    orders[numOrders].arrivalTime = arrivalTime; // 예상 도착 시간 저장
    numOrders++; // 주문 수 증가
}

// 주문 내역을 저장하는 함수
void saveOrder(const char* filename, const char* name, const char* address, const char* menu, int count, const char* time, int arrivalTime) {
    FILE* orderFile;
    orderFile = fopen(filename, "a");
    if (orderFile == NULL) {
        printf("파일을 열 수 없습니다.\n");
        return;
    }

    // 주문 내역 생성 및 파일에 저장
    fprintf(orderFile, "주문시간: %s, 예상도착시간: %02d:%02d, 주문자: %s, 주소: %s, 메뉴: %s, 수량: %d\n", time, arrivalTime / 60, arrivalTime % 60, name, address, menu, count);

    fclose(orderFile);
}

// 문자열 형태의 시간을 분 단위로 변환하는 함수
int timeConverter(const char* timeStr) {
    int hour, min;
    sscanf(timeStr, "%d:%d", &hour, &min);
    return hour * 60 + min; // 분 단위로 변환하여 반환
}

void completeDelivery(Order* order) {
    // 주문 상태를 "배달 완료"로 변경
    order->status = DELIVERED;

    // 화면에 배달이 완료되었다는 메시지 출력
    printf("========================\n");
    printf("배달이 완료되었습니다!!\n");
    printf("이름: %s\n", order->name);
    printf("메뉴: %s\n", order->menu);
    printf("수량: %d\n", order->count);
    printf("========================\n");
}

// 주문하고 배달까지 남은 시간을 출력하는 함수
void remainingTime() {
    char name[100];    // 이름을 저장할 문자열 배열
    char address[100]; // 주소를 저장할 문자열 배열

    printf("이름: ");
    scanf("%s", name);
    printf("주소: ");
    scanf("%s", address);

    // 주문 리스트에서 이름과 주소가 일치하는 주문을 찾기
    Order* order = findOrder(name, address);
    if (order != NULL) {
        // 현재 상태 출력
        printf("현재 상태: %s\n", getOrderStatusString(order->status));

        // 주문이 있는 경우 배달까지 남은 시간 출력
        int arrivalTime = order->arrivalTime;

        // 현재 시간 가져오기
        time_t now;
        time(&now);
        struct tm* local = localtime(&now);
        int currentTime = local->tm_hour * 60 + local->tm_min;

        // 주문 상태가 "배달 완료"인 경우에는 배달이 완료되었다는 메시지와 주문 내용 출력
        if (order->status == DELIVERED) {
            completeDelivery(order);
        }
        else if (currentTime >= arrivalTime) {
            completeDelivery(order);
        }
        else {
            // 주문 상태가 "조리 중"이거나 "배달 중"인 경우에만 남은 시간 출력
            int remainingTime = (arrivalTime - currentTime + 24 * 60) % (24 * 60);

            printf("주문하신 음식의 배달까지 남은 시간은 약 %d분입니다.\n", remainingTime);
        }
    }
    else {
        // 주문이 없는 경우 메시지 출력
        printf("주문 내역을 찾을 수 없습니다.\n");
    }
}

// 주문을 받고 주문 내역을 저장하는 함수
void getOrderAndSave(int store) {
    char name[100];    // 이름을 저장할 문자열 배열
    char address[100]; // 주소를 저장할 문자열 배열
    char menu[100];    // 메뉴를 저장할 문자열 배열
    int count;         // 수량을 저장할 변수

    char filename[100]; // 파일 이름을 저장할 문자열 배열

    // 가게 영업 시간 확인
    time_t currentTime;
    time(&currentTime);
    struct tm* localTime = localtime(&currentTime);
    int currentHour = localTime->tm_hour;
    int currentMin = localTime->tm_min;
    int currentMinutes = currentHour * 60 + currentMin;

    switch (store) {
    case 1:
        // 우왕치킨은 18시부터 24시까지 영업
        if (currentHour < 18 || (currentHour == 24 && currentMin != 0)) {
            printf("현재 영업 준비 중입니다.\n");
            return;
        }
        strcpy(filename, "우왕치킨주문리스트.txt");
        break;
    case 2:
        // 우왕 중식은 10시부터 20시까지 영업
        if (currentHour < 10 || (currentHour == 20 && currentMin != 0)) {
            printf("현재 영업 준비 중입니다.\n");
            return;
        }
        strcpy(filename, "우왕중식주문리스트.txt");
        break;
    case 3:
        // 우왕냉면은 12시부터 21시까지 영업
        if (currentHour < 12 || (currentHour == 21 && currentMin != 0)) {
            printf("현재 영업 준비 중입니다.\n");
            return;
        }
        strcpy(filename, "우왕냉면주문리스트.txt");
        break;
    case 4:
        // 우왕족발은 17시부터 05시까지 영업
        if ((currentHour < 17 && currentHour >= 5) || (currentHour == 17 && currentMin != 0)) {
            printf("현재 영업 준비 중입니다.\n");
            return;
        }
        strcpy(filename, "우왕족발주문리스트.txt");
        break;
    case 5:
        // 우왕국밥은 24시간 영업
        strcpy(filename, "우왕국밥주문리스트.txt");
        break;
    case 6:
        // 우왕덮밥은 09시부터 19시까지 영업
        if (currentHour < 9 || (currentHour == 19 && currentMin != 0)) {
            printf("현재 영업 준비 중입니다.\n");
            return;
        }
        strcpy(filename, "우왕덮밥주문리스트.txt");
        break;
    default:
        printf("잘못된 가게 번호입니다.\n");
        return;
    }

    while (1) {
        printf("\n주문하실 정보를 입력해주세요!\n");
        printf("이름: ");
        scanf("%s", name);
        printf("주소: ");
        scanf("%s", address);

        // 입력된 주소가 가능한 구역인지 확인
        if (!isAddressValid(address)) {
            printf("배달 가능한 지역이 아닙니다.\n");
            printf("배달 가능 지역: ");
            for (int i = 0; i < sizeof(availableAreas) / sizeof(availableAreas[0]); ++i) {
                printf("%s, ", availableAreas[i]);
            }
            printf("\n");
        }
        else {
            break; // 유효한 주소를 입력했을 경우 반복문 탈출
        }
    }

    while (1) {
        printf("메뉴: ");
        scanf("%s", menu);

        // 메뉴가 존재하는지 확인
        bool validMenu = false;
        for (int i = 0; i < sizeof(menuNames) / sizeof(menuNames[0]); ++i) {
            if (strcmp(menu, menuNames[i]) == 0) {
                validMenu = true;
                break;
            }
        }

        if (!validMenu) {
            printf("존재하지 않는 메뉴입니다.\n");
            continue;
        }

        break; // 유효한 메뉴를 입력했을 경우 반복문 탈출
    }

    while (1) {
        printf("수량: ");
        if (scanf("%d", &count) != 1 || count <= 0) {
            printf("올바르지 않은 수량입니다.\n");
            // 입력 버퍼 비우기
            while (getchar() != '\n');
            continue;
        }

        break; // 유효한 수량을 입력했을 경우 반복문 탈출
    }

    // 저장된 주문 내역 출력
    printf("주문하신 정보: 이름(%s), 주소(%s), 메뉴(%s), 수량(%d)\n", name, address, menu, count);

    // 현재 시간 가져오기
    time_t now;
    time(&now);
    struct tm* local = localtime(&now);

    // 배달 예상 시간 계산
    int deliveryTime = calculateDeliveryTime(address, menu);
    // 예상 도착 시간 계산
    int arrivalTime = (local->tm_hour * 60 + local->tm_min + deliveryTime) % (24 * 60); // 24시간 기준으로 나머지를 구해서 계산
    int arrivalHour = arrivalTime / 60;
    int arrivalMin = arrivalTime % 60;

    printf("예상 도착 시간: %02d:%02d\n", arrivalHour, arrivalMin);

    // 주문 내역을 배열에 추가
    addOrder(name, address, menu, count, asctime(local), arrivalTime); // 현재 시간 문자열 사용

    // 주문 저장
    saveOrder(filename, name, address, menu, count, asctime(local), arrivalTime);
}

int main() {
    int menu;

    while (1) {
        printMenu();
        scanf("%d", &menu);

        switch (menu) {
        case 1:
            printf("================================================\n\n");
            printf("                   <가게 목록>\n\n");
            printf("      1.우왕치킨    2.우왕중식   3.우왕냉면\n\n");
            printf("      4.우왕족발    5.우왕국밥   6.우왕덮밥\n\n\n");
            printf("================================================\n");
            printf("주문할 가게를 선택해주세요: ");

            int store;
            scanf("%d", &store);

            switch (store) {
            case 1:
                printStoreMenu("우왕치킨.txt");
                getOrderAndSave(store);
                break;
            case 2:
                printStoreMenu("우왕중식.txt");
                getOrderAndSave(store);
                break;
            case 3:
                printStoreMenu("우왕냉면.txt");
                getOrderAndSave(store);
                break;
            case 4:
                printStoreMenu("우왕족발.txt");
                getOrderAndSave(store);
                break;
            case 5:
                printStoreMenu("우왕국밥.txt");
                getOrderAndSave(store);
                break;
            case 6:
                printStoreMenu("우왕덮밥.txt");
                getOrderAndSave(store);
                break;
            default:
                printf("잘못된 가게 번호입니다.\n");
                break;
            }
            break;
        case 2:
            remainingTime();
            break;
        case 3:
            printf("프로그램을 종료합니다.\n");
            return 0;
        default:
            printf("잘못된 입력입니다. 다시 입력해주세요.\n");
            break;
        }
    }

    // 동적 할당된 메모리 반환
    free(orders);

    return 0;
}