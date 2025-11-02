/**
 * Questo script legge le environment variables definite nell'ambiente e le scrive in un file env.js
 * @param target Accetta un path personalizzato in cui generare il file env.js. Default: src/assets/env.js
 */

import { writeFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';
import { dirname, join } from 'node:path';

// Costruzione dell’oggetto env
const env = {
  BRIDGE_ADDRESS: process.env.BRIDGE_ADDRESS ?? 'localhost',
  BRIDGE_HTTP_PORT: process.env.BRIDGE_HTTP_PORT ?? '3001',
  BRIDGE_WS_PORT: process.env.BRIDGE_WS_PORT ?? '3002'
};

console.log(`Generating env.js...`);

// Otteniamo il nome della directory attuale
const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

// Genera il contenuto del file
const content = `globalThis.__env = ${JSON.stringify(env, null, 2)};\n`;

// Usa l'argomento CLI, altrimenti il percorso di default
const target = process.argv[2] ?? join(__dirname, '../src/assets/env.js');

try {
  await writeFile(target, content, { encoding: 'utf-8' });
  console.log(`Successfully generated env.js at ${target}`);
  console.table(env);
} catch (error) {
  console.error('Error generating env.js:', error);
  process.exit(1);
}
