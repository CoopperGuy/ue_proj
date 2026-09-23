"""Small external client for the project's native Unreal MCP server.

DataTable writes are separate MCP requests, outside ProgrammaticToolset's Undo
transaction. All editor/asset operations remain native MCP tool calls.
"""
import argparse
import http.client
import json
from pathlib import Path


class UnrealMCP:
    def __init__(self, timeout=60):
        self.url = 'http://127.0.0.1:8010/mcp'
        self.session = None
        self.next_id = 0
        self.protocol = '2025-11-25'
        self.timeout = timeout
        result = self.request('initialize', {
            'protocolVersion': self.protocol, 'capabilities': {},
            'clientInfo': {'name': 'ARPG-Skill-Recovery', 'version': '1.0'}})
        self.protocol = result['protocolVersion']
        self.request('notifications/initialized', {}, notification=True)

    def request(self, method, params, notification=False):
        self.next_id += 1
        body = {'jsonrpc': '2.0', 'method': method, 'params': params}
        if not notification:
            body['id'] = self.next_id
        headers = {'Content-Type': 'application/json',
                   'Accept': 'application/json, text/event-stream'}
        if self.session:
            headers['Mcp-Session-Id'] = self.session
            headers['MCP-Protocol-Version'] = self.protocol
        connection = http.client.HTTPConnection('127.0.0.1', 8010, timeout=self.timeout)
        connection.request('POST', '/mcp', body=json.dumps(body).encode('utf-8'), headers=headers)
        try:
            response = connection.getresponse()
            if response.status >= 400:
                raise RuntimeError('MCP HTTP error ' + str(response.status) + ': ' + response.read().decode('utf-8'))
            self.session = response.headers.get('Mcp-Session-Id', self.session)
            if notification:
                response.read()
                return None
            if 'text/event-stream' in response.headers.get('Content-Type', ''):
                payload = None
                preview = []
                for line in response:
                    if len(preview) < 3:
                        preview.append(line[:250].decode('utf-8', errors='replace'))
                    if line.startswith(b'data:'):
                        candidate = json.loads(line[5:].strip())
                        if str(candidate.get('id')) == str(body['id']):
                            payload = candidate
                            break
                if payload is None:
                    raise RuntimeError('MCP stream ended without the matching response: ' + repr(preview))
            else:
                payload = json.load(response)
        finally:
            connection.close()
        if 'error' in payload:
            raise RuntimeError(json.dumps(payload['error'], ensure_ascii=False))
        return payload['result']

    def tool(self, name, args):
        result = self.request('tools/call', {'name': name, 'arguments': args})
        if result.get('isError'):
            raise RuntimeError(json.dumps(result, ensure_ascii=False))
        content = result.get('content', [])
        text = '\n'.join(c['text'] for c in content if c.get('type') == 'text')
        return json.loads(text) if text else None

    def call(self, group, name, args):
        result = self.tool('call_tool', {'toolset_name': group, 'tool_name': name, 'arguments': args})
        return result.get('returnValue') if isinstance(result, dict) else result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--describe')
    parser.add_argument('--call', nargs=2, metavar=('TOOLSET', 'TOOL'))
    parser.add_argument('--args-file', type=Path)
    parser.add_argument('--script', type=Path, help='Read-only editor smoke script, or asset setup script; never DataTable writes')
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    client = UnrealMCP()
    if args.describe:
        result = client.tool('describe_toolset', {'toolset_name': args.describe})
    elif args.call:
        values = json.loads(args.args_file.read_text(encoding='utf-8')) if args.args_file else {}
        result = client.call(*args.call, values)
    elif args.script:
        if args.script.name == 'register_elemental_pack.py':
            raise RuntimeError('DataTable registration must use separate MCP requests')
        result = client.call('editor_toolset.toolsets.programmatic.ProgrammaticToolset',
                             'execute_tool_script', {'script': args.script.read_text(encoding='utf-8')})
    else:
        result = client.request('tools/list', {})
    if isinstance(result, str):
        try:
            result = json.loads(result)
        except json.JSONDecodeError:
            pass
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding='utf-8')
        print(json.dumps({'output': str(args.output), 'passed': result.get('passed') if isinstance(result, dict) else None}))
    else:
        print(json.dumps(result, ensure_ascii=True))


if __name__ == '__main__':
    main()
