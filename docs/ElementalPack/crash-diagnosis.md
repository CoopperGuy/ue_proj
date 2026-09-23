# 등록 중 에디터 종료 원인

2026-09-22 로컬 크래시 로그 열람을 사용자에게 허용받아 확인했습니다.

`Saved/Logs/ARPG_2.log`의 2026.09.22-07.16.49 UTC 기록:

```
Script Stack:
/Script/Engine.DataTableFunctionLibrary.FillDataTableFromJSONString
/EditorToolset/Python/editor_toolset/toolsets/data_table_PY.DataTableTools.add_rows
/Script/ToolsetRegistry.ToolsetRegistry.ExecuteTool
Assertion failed: !IsRooted()
```

호출 스택은 `UCSVImportFactory::FactoryCreateText` → `StaticConstructObject_Internal` → `UObjectBaseUtility::MarkAsGarbage`입니다. 에셋 손상이나 스킬 수치 오류를 나타내는 로그가 아니라 테이블 객체 재생성과 Undo 트랜잭션의 충돌입니다.

확인한 엔진 코드:

- `EditorToolset/.../data_table.py`: add_rows/set_rows가 모두 `_write_table` → `fill_from_json_string`으로 테이블 전체를 재가져옵니다.
- `EditorToolset/.../programmatic.py`: execute_tool_script가 `_TransactionalScriptRunner`를 사용해 전체 스크립트에 Undo 트랜잭션을 엽니다.
- `CSVImportFactory.cpp:315`: 기존 테이블을 같은 이름의 NewObject로 재생성합니다.
- `UObjectGlobals.cpp:4864`: GUndo가 활성화된 transactional 객체 생성 시 MarkAsGarbage를 호출합니다.
- `UObjectBaseUtility.h:209`: rooted 객체의 MarkAsGarbage를 금지하는 assertion입니다.
- `ModelContextProtocolToolsetRegistryAdapter.cpp:258`: 개별 네이티브 call_tool은 ToolsetRegistry로 직접 전달됩니다. ProgrammaticToolset의 외부 트랜잭션을 만들지 않습니다.

수정: 테이블 쓰기는 외부 클라이언트 `register_elemental_pack_native.py`에서 개별 네이티브 MCP 요청을 순차 실행합니다. 테이블마다 저장하고 실패하면 즉시 중단합니다. 기존 ProgrammaticToolset용 Python 등록 진입점은 오류를 반환하도록 차단했습니다. 읽기 전용 에디터 검사는 기존 ProgrammaticToolset을 계속 사용합니다. 엔진 코드, root 플래그, 전역 Undo 상태는 변경하지 않았습니다.

2026-09-22 수정 경로로 스킬 3개와 variant 9개, WarriorSkillSet 등록·저장에 성공했습니다. 등록 전 기존 스킬 20행·variant 23행의 내용 보존을 확인했고, 새 에디터 검사 174개와 기존 서리 창 검사 36개가 모두 통과했습니다. `registration-result.json` 및 검사 결과 JSON에서 확인할 수 있습니다. 이 결과는 에디터 등록 경로의 검증이며 C++ 빌드나 전투 검증을 의미하지 않습니다.
