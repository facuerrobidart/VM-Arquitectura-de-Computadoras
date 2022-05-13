// deno compile --allow-read parser.ts

interface Directive {
  segment: string,
  size: string
}

interface Const {
  name: string,
  value: string
}

interface Line {
  label?: string,
  mnemonic?: string,
  operands: string[],
  comment?: string,
  directive?: Directive,
  const?: Const
}

function parse(line: string): Line {
  const strings = line.match(/".*"/) || [];
  const strKeys = strings.map((_, index) => `${index}string`);

  const tempLine = strings.reduce((line, str, index) =>
    line.replace(str, `${strKeys[index]}`), line);

  const [instruction, tempComment] = tempLine.split(';');
  const tempTokens = instruction.trimStart().split(/(?<![^'].')[\s,]+/);

  const restoreStrings = (text: string) =>
    strKeys.reduce((text, key, index) =>
      text?.replace(key, strings[index]), text);

  const tokens = tempTokens.map(restoreStrings);
  const comment = restoreStrings(tempComment);

  if (tokens[0]?.startsWith('\\\\')) {
    return {
      operands: [],
      comment,
      directive: {
        segment: tokens[0].slice(2),
        size: tokens[1]
      }
    }
  }

  if (tokens[1]?.toUpperCase() === 'EQU') {
    return {
      operands: [],
      comment,
      const: {
        name: tokens[0],
        value: tokens[2]
      }
    }
  }

  return {
    label: tokens[0]?.endsWith(':') ? tokens.shift()?.slice(0, -1) : undefined,
    mnemonic: tokens[0] || undefined,
    operands: tokens.slice(1).filter(token => token.length),
    comment
  };
}

const text = Deno.readTextFileSync(Deno.args[0]);
const line = text.split(/\r\n|\r|\n/)[0];
const parsed = parse(line);

console.log(parsed.label ?? '');
console.log(parsed.mnemonic ?? '');
console.log(parsed.operands[0] ?? '');
console.log(parsed.operands[1] ?? '');
console.log(parsed.comment ?? '');
console.log(parsed.directive?.segment ?? '');
console.log(parsed.directive?.size ?? '');
console.log(parsed.const?.name ?? '');
console.log(parsed.const?.value ?? '');