# Project Knowledge Base & AI Guidelines (Arknights: Endfield Style Mini-Project)

이 문서는 프로젝트 전반의 기술 스택, 코딩 컨벤션, 아키텍처 원칙을 정의하여 AI 어시스턴트가 항상 일관된 컨텍스트를 유지하도록 돕는 상시 규칙(Always-on Rules)입니다.

---

## 1. 프로젝트 개요 (Overview)
* **프로젝트명**: Mini_Endfield
* **주요 목적**: **Arknights: Endfield**(명일방주: 엔드필드)의 차세대 고성능 렌더링 프레임워크를 Unity 6.5 + C++ Vulkan 환경에서 재현 및 벤치마크
* **핵심 타겟**: 500,000 ~ 600,000개 수준의 대규모 인스턴스를 초당 60fps 이상으로 부드럽게 시뮬레이션 및 렌더링

---

## 2. 기술 스택 & 환경 (Tech Stack & Environment)
* **엔진**: Unity 6.5 (Entities 1.3 / DOTS, Burst Compiler, Mathematics, Collections, Universal RP)
* **언어**: C# (Managed / DOTS ECS), C++17 (Native Graphics Core)
* **그래픽스 백엔드**: Vulkan API (Explicit Low-Level Device Abstraction, SPIR-V Shaders)
* **빌드 시스템**: CMake 3.15+, MSVC / Clang (NativeCore)

---

## 3. 핵심 렌더링 4대 기둥 (4 Core Architectural Pillars)

### ① Custom ECS & Data-Oriented Design
* SoA(Structure of Arrays) 방식의 청크(Chunk) 메모리 배치를 통한 CPU 캐시 적중률 극대화.
* 런타임 타입 등록, 리플렉션, 해시맵 조회를 배제하고 128비트 고정 마스크와 포인터 산술 연산으로 3사이클 내 접근.
* 구조적 변경은 일괄 Swap-and-Pop으로 처리하여 엔티티당 연산 단가 최소화.

### ② 통합 컬링 / 64비트 정렬 / 제로 할당 배칭 (Culling, Sort, Batch)
* **뷰 통합 컬링**: 다중 뷰(메인 뷰, 섀도우 캐스케이드, 리플렉션 등)의 프러스텀 테스트를 단일 패스로 병렬 실행하여 가시성 공유.
* **소프트웨어 오클루전 컬링**: GPU 저해상도 뎁스 버퍼 + CPU 타일 분할(8타일) 락프리(Lock-Free) 래스터라이제이션.
* **64비트 정렬 키(`SortKey`)**: `[Pass ID | Pipeline ID | Material/Mesh ID | Distance]`를 인코딩하여 표준 정렬(`std::sort`) 한 번으로 배칭 완료.
* **제로 할당 배칭**:
  - Set 0: 패스당 1회 (카메라/라이팅)
  - Set 1: 머티리얼당 1회 (텍스처/상수 캐싱)
  - Set 2: 드로우당 (거대한 단일 Dynamic Uniform Buffer + Dynamic Offset 이동)
  - 워커 스레드 `0x7F7F7F7F` 플레이스홀더를 활용한 병렬 커맨드 빌드 및 중복 바인딩 스킵.

### ③ C++ 네이티브 렌더 파이프라인 & 렌더 그래프
* 매 프레임 수천 번 호출되는 핵심 렌더 루프에서 C# 언어 간 바인딩(P/Invoke) 오버헤드를 배제하고 순수 C++로 제어.
* 렌더 그래프: 프론트엔드 선언 시점에 타입 태그 기반으로 읽기/쓰기 자원을 명시 → 컴파일 타임에 배리어 접근 마스크 자동 수집.
* 같은 플러시(Flush) 내 모든 해저드를 단일 파이프라인 배리어로 병합(Barrier Merging).

### ④ Vulkan 저수준 디바이스 추상화 계층
* 고수준 범용 추상화 대신 렌더 패스, 배리어, 디스크립터 셋을 외부에 명시적으로 노출하여 폴백 없는 예측 가능한 극대화된 성능 달성.

---

## 4. 벤치마크 목표 수치 (Benchmark Target Metrics - Unite Seoul 2026)
* **후보군 오브젝트 처리**: 모바일 50만 개, PC 60만 개
* **컬링 압축 비율**: 60만 개 후보 중 약 90% 컬링 압축 (실제 렌더링 약 6만 개)
* **정렬 & 배칭 소요 시간 (Sort + Batching Budget)**:
  * **PC: 1.0 ms 이내 방어 (핵심 지표)**
  * **모바일: 3.0 ms 이내 방어**

---

## 5. 코딩 표준 및 시스템 규칙
1. **Zero-GC & Burst 호환**: DOTS 런타임 시스템 내 모든 데이터 조작은 `NativeArray`, `Unsafe`, Job System을 사용하며 GC Alloc을 절대 유발하지 않는다.
2. **DLL Lock 방지**: NativeCore 빌드 시 DLL 버전 네이밍(예: `MiniEndfieldVulkanPlugin_v12.dll`)을 통해 Unity 에디터 파일 락을 방지한다.
3. **독립 벤치마크**: GPU 창 없이도 알고리즘 성능을 측정할 수 있는 헤드리스 벤치마크와 인게임 실시간 HUD 오버레이를 동시에 지원한다.

