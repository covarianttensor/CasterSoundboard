#!/usr/bin/env node

/**
 * Caster Soundboard MCP Stdio Bridge
 * 
 * Provides standard Model Context Protocol (MCP) stdio JSON-RPC 2.0 communication
 * for AI assistants (Antigravity, Claude Desktop, Cursor, etc.), connecting to
 * Caster Soundboard's native MCP server on port 5055.
 * 
 * Usage in MCP client config (e.g. claude_desktop_config.json):
 * {
 *   "mcpServers": {
 *     "caster": {
 *       "command": "node",
 *       "args": ["/path/to/CasterSoundboard/scripts/caster-mcp.js"]
 *     }
 *   }
 * }
 */

const readline = require('readline');
const http = require('http');

const MCP_PORT = process.env.CASTER_MCP_PORT || 5055;
const MCP_HOST = process.env.CASTER_MCP_HOST || '127.0.0.1';

function sendToCaster(jsonObj) {
  return new Promise((resolve, reject) => {
    const postData = JSON.stringify(jsonObj);
    const req = http.request({
      hostname: MCP_HOST,
      port: MCP_PORT,
      path: '/message',
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Content-Length': Buffer.byteLength(postData)
      },
      timeout: 5000
    }, (res) => {
      let body = '';
      res.on('data', chunk => body += chunk);
      res.on('end', () => {
        try {
          resolve(JSON.parse(body));
        } catch (e) {
          reject(new Error(`Invalid JSON response from Caster MCP server: ${body}`));
        }
      });
    });

    req.on('error', (err) => {
      reject(err);
    });

    req.on('timeout', () => {
      req.destroy();
      reject(new Error('Request to Caster MCP server timed out'));
    });

    req.write(postData);
    req.end();
  });
}

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout,
  terminal: false
});

rl.on('line', async (line) => {
  const trimmed = line.trim();
  if (!trimmed) return;

  let request;
  try {
    request = JSON.parse(trimmed);
  } catch (err) {
    const errorResponse = {
      jsonrpc: '2.0',
      id: null,
      error: { code: -32700, message: 'Parse error' }
    };
    process.stdout.write(JSON.stringify(errorResponse) + '\n');
    return;
  }

  try {
    const response = await sendToCaster(request);
    process.stdout.write(JSON.stringify(response) + '\n');
  } catch (err) {
    // If server is not running or unreachable, return a helpful MCP JSON-RPC error
    const errorResponse = {
      jsonrpc: '2.0',
      id: request.id !== undefined ? request.id : null,
      error: {
        code: -32000,
        message: `Caster Soundboard MCP server connection error: ${err.message}. Is Caster Soundboard running with MCP server enabled on port ${MCP_PORT}?`
      }
    };
    process.stdout.write(JSON.stringify(errorResponse) + '\n');
  }
});
