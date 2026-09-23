# Elemental Pack

## 제작한 콘텐츠

| ID | 스킬 | 기본 동작 | 전용 variant |
|---|---|---|---|
| 1013 | 용암 분출 | 반경 240cm, 4초 장판, 0.5초 간격 8회, 회당 기본 피해 28 | 범위 +35%, 쿨다운 -25%, 마나 -30% |
| 1014 | 전격 산탄 | 탄환 3개, 간격 12도, 속도 2300cm/s, 탄환당 기본 피해 36 | 탄환 +2, 최대 3명 관통, 적중 시 2개 분열 |
| 1015 | 서리 돌진 | 600cm/0.25초 돌진 후 반경 220cm 서리 장판, 1.8초/3회, 회당 기본 피해 55 | 범위 +40%, 적중 시 1.2초 빙결, 쿨다운 -25% |

variant ID는 24~32입니다. 세 스킬과 전용 variant를 데이터 테이블에 등록하고 WarriorSkillSet 보상 풀에도 추가·저장했습니다. 시작 스킬이나 퀵슬롯은 변경하지 않았습니다.

## 실행 구조

앞서 만든 서리 창을 포함해 네 스킬 모두 `AbilityClass = UDiaGameplayAbility`, 최상위 `ExtraData = {}`이며 실행 동작은 `Steps`에만 작성했습니다. 저장 후 네이티브 MCP로 조회한 값은 `tables-after-registration.json`에 있습니다.

| 스킬 | Steps 순서 |
|---|---|
| 서리 창 (1012) | ProjectileSpawn |
| 용암 분출 (1013) | GroundSpawn |
| 전격 산탄 (1014) | ProjectileSpawn |
| 서리 돌진 (1015) | Charge → GroundSpawn |

`UDiaGameplayAbility::RunNextStep`이 해당 StepExecutor를 실행합니다. `BP_FrostLance` 등의 BP는 Step이 생성할 월드 Actor를 지정하며, 실행 순서를 구현하는 전용 Ability가 아닙니다. 발사체·장판 Actor와 Step 내부의 ProjectileData/GroundData 구조체는 이 구조에서도 계속 사용합니다.

신규 스킬은 단일 동작도 Steps에 작성합니다. `GAS/Abilities`의 기존 전용 Ability 클래스는 호환 유지용이며 신규 사용을 지양합니다. 새로운 동작이 필요하면 StepData/Executor를 확장합니다. 스킬 지급 치트는 테이블에 지정된 Ability를 지급하고 퀵슬롯을 연결하므로 이 실행 구조를 바꾸지 않습니다.

## 현재 검증 상태

Blender에서 모델 3개와 아이콘 12개를 생성하고 렌더를 확인했습니다. 원본은 프로젝트 `ArtSource/ElementalPack`에 있습니다.

Unreal MCP를 통해 메시·아이콘 가져오기, 머티리얼 9개와 Niagara 6개 생성, 스킬 블루프린트 3개 및 상태 효과 2개 설정까지 응답을 확인했습니다. 블루프린트 에셋 컴파일도 성공했습니다.

2026-09-22 새 MCP 세션으로 연결해 테이블 등록을 완료했습니다. 스킬 테이블은 총 23행, variant 테이블은 총 32행이며, 등록 전 기존 스킬 20행과 variant 23행이 동일하게 유지됐습니다. 직업 테이블의 기존 스킬 순서도 유지했습니다.

새 에디터 스모크 검사 **174/174**, 기존 서리 창 회귀 검사 **36/36**이 통과했습니다. 새 에셋 35개와 테이블 3개의 저장 상태, 스킬·효과 연결, Niagara 컴파일 상태, 수치, 중복 ID, 기존 감속 효과의 SetByCaller 맵을 확인했습니다. 결과는 `editor-smoke-result.json`, `frost-lance-regression-result.json`, `registration-result.json`에 있습니다.

검사 중 발견한 새 BP 3개의 기본 수명은 각각 4초, 1.2초, 1.8초로 수정하고 BP 컴파일·저장을 완료했습니다. 저장 여부 검사는 네이티브 도구가 요구하는 패키지 경로를 사용하도록 수정했습니다.

이전 등록 중 크래시는 테이블 재가져오기와 외부 Undo 트랜잭션의 충돌이었습니다. 테이블 쓰기를 개별 네이티브 MCP 요청으로 분리한 뒤 등록·저장에 성공했습니다. 자세한 근거는 `crash-diagnosis.md`에 있습니다. C++ 빌드·실제 전투 테스트는 실행하지 않았습니다.

## 연결 복구 후 순서

1. ARPG 에디터의 MCP를 8010에서 켜고 리스너의 프로세스를 확인합니다.
2. `assets.json` 목록을 AssetTools로 확인합니다. `.uasset` 파일을 직접 읽거나 수정하지 않습니다.
3. 누락된 항목만 제작 스크립트로 복원합니다. importer는 이미 존재하는 메시·아이콘·BP를 건너뜁니다. 머티리얼·Niagara 생성 스크립트는 첫 생성용이므로 해당 에셋이 존재하면 생성 단계를 건너뛰고 설정을 확인해야 합니다.
4. 등록이 필요한 경우 `py -3 Tools/Unreal/register_elemental_pack_native.py`를 Source 폴더에서 실행합니다. 이 외부 Python 클라이언트는 8010의 네이티브 MCP에 개별 요청을 보냅니다. ID 충돌·PIE 실행 여부·쿨다운 태그를 검사하고, 등록 전 테이블 스냅샷을 남기며, 기존 행의 보존을 확인한 뒤 저장합니다. ProgrammaticToolset 안에서 테이블을 쓰는 기존 `register_elemental_pack.py`는 크래시 방지를 위해 차단했습니다.
5. `save_elemental_pack.py`로 새 에셋 35개와 테이블 3개만 저장합니다.
6. `test_elemental_pack_editor.py`, 기존 `test_frost_lance_editor.py`를 실행하고 결과를 이 폴더에 저장합니다. `tables-before.json`과 비교해 기존 스킬·variant 행이 유지됐는지도 검사합니다.

Tools/Unreal의 제작·검사 스크립트는 `execute_tool`을 주입하는 Unreal 네이티브 ProgrammaticToolset에서 실행합니다. 외부 클라이언트로 다음과 같이 호출할 수 있습니다. 테이블 등록은 위의 별도 명령을 사용합니다.

```powershell
py -3 Tools/Unreal/native_mcp_client.py --script Tools/Unreal/test_elemental_pack_editor.py --output docs/ElementalPack/editor-smoke-result.json
py -3 Tools/Unreal/native_mcp_client.py --script Tools/Unreal/test_frost_lance_editor.py --output docs/ElementalPack/frost-lance-regression-result.json
```

데이터 테이블 등록은 플레이 중인 캐릭터에게 즉시 스킬을 지급하는 작업과 다릅니다. 현재 보상 풀에는 등록됐지만 시작 스킬이나 퀵슬롯을 교체하지 않았으며, 실행 중 캐릭터의 소유 스킬 상태는 이 검사 범위에 포함되지 않습니다.

## C++ 변경 및 전투 검증

`DiaGroundObj.cpp`에서 일반 GroundSpawnStep의 지속 타격 타이머를 시작하고, 범위 배율을 메시와 Niagara 컴포넌트에도 적용하도록 보완했습니다. 한 틱에 같은 대상의 여러 충돌체를 중복 타격하지 않으며, 기존 타깃 필터를 사용해 아군을 제외합니다. 상시 디버그 구 표시도 제거했습니다.

공통 장판 코드 변경이므로 기존 IceRain/BurningGround도 회귀 검사가 필요합니다. C++ 빌드 후 용암 8틱, 서리 3틱, 같은 적의 다중 충돌체, 아군 제외, 범위 variant의 시각/피격 일치, 벽 앞 돌진과 도착 지점, 빙결 해제 후 이동 속도 복원, 산탄·관통·분열 조합을 확인해야 합니다. 서리 장판 시각 컴포넌트의 발밑 오프셋은 현재 -88cm이며 캐릭터 캡슐 높이가 다르면 조정이 필요합니다.

기존 `GE_IceSlow2s_half`의 지속시간이 태그 없는 SetByCaller로 설정돼 있어 새 `GE_FrostRush_Slow`를 복제하고 고정 2초로 지정했습니다. 원본 효과는 변경하지 않았습니다.

프로젝트 AGENTS.md의 빌드 경계에 따라 사용자 요청 없이 Unreal/UBT를 실행하지 않았습니다. 이전 서리 창 관통 수정과 이번 장판 수정은 C++ 컴파일 이후 실행 파일에 반영됩니다.
