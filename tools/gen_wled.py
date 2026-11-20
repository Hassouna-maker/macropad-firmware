#!/usr/bin/env python3
"""Copies WLED effect code (FX.cpp, EUPL-1.2) into the macropad project headers.
usage: gen_wled.py <wled00 dir> <out dir>
Effects are chosen with the metadata flags in WLED's own effect strings:
  flags with 'v' or 'f' = music reactive -> skipped. '2' only = matrix effect. '1' = strip/stick effect.
"""
import re, sys, os, json
src, out = sys.argv[1], sys.argv[2]
L = open(os.path.join(src, 'FX.cpp'), encoding='utf-8').read().split('\n')

# ---- 1. metadata strings and the addEffect table
meta = {}
for ln in L:
    m = re.match(r'static const char (_data_FX_MODE_\w+)\[\] PROGMEM = "(.*?)";', ln)
    if m: meta[m.group(1)] = m.group(2)
adds = []
for ln in L:
    m = re.match(r'\s*addEffect\((FX_MODE_\w+),\s*&(\w+),\s*(_data_FX_MODE_\w+)\);', ln)
    if m: adds.append((m.group(1), m.group(2), m.group(3)))

adds.insert(0, ('FX_MODE_STATIC', 'mode_static', '_data_FX_MODE_STATIC'))   # registered differently in WLED
def flags_of(md):
    p = md.split(';')
    return p[3] if len(p) > 3 else ''
def name_of(md): return md.split('@')[0].split(';')[0]

# ---- 2. chunk the file between two line numbers
def chunks(a, b):
    """a,b are 1-based inclusive. returns list of (first,last,text) at top level"""
    res = []; depth = 0; start = a - 1; i = a - 1
    while i < b:
        ln = L[i]
        code = re.sub(r'//.*', '', ln)
        code = re.sub(r'"(\\.|[^"\\])*"', '""', code)
        code = re.sub(r"'(\\.|[^'\\])'", "''", code)
        depth += code.count('{') - code.count('}')
        s = ln.strip()
        end = False
        if depth == 0:
            if s.startswith('#'): end = True
            elif s.endswith('}') or s.endswith('};') : end = True
            elif s.startswith('static const char _data_') or s.endswith(';') and not s.startswith('//') and not s.startswith('*') : end = True
        if end and not (s.startswith('/*') and '*/' not in s):
            res.append((start + 1, i + 1, '\n'.join(L[start:i + 1]))); start = i + 1
        i += 1
    if start < b: res.append((start + 1, b, '\n'.join(L[start:b])))
    return res

BAD = re.compile(r'getAudioData|um_data|volumeSmth|fftResult|SEGMENT\.fft|simulateSound|strip\.getSegment\b|Segment\s*&|'
                 r'drawCharacter|PartSys|ParticleSystem|SEGMENT\.soundSim|FFT_|UsermodManager|loadImage|openFile|WLED_FS|'
                 r'strip\.getPixelColor|getPixelColorNoMap|fontmanager|'
                 r'getSegmentsNum|SEGMENT\.move|strip\.ino')
def nocomment(text): return re.sub(r'//[^\n]*|/\*.*?\*/', '', text, flags=re.S)
def func_name(text):
    m = re.search(r'^\s*(?:static\s+)?(?:\[\[\w+::\w+\]\]\s*)?(?:void|uint16_t|uint8_t|bool|int|float|uint32_t)\s+(\w+)\s*\(', text, re.M)
    return m.group(1) if m else None

want = {'1': [], '2': []}
# left out on purpose: Sunrise takes an hour to get bright, Solid is already 'static', Copy/Image need WLED segments/files
SKIP_NAMES = {'Sunrise', 'Copy Segment', 'Image', 'Solid'}
modefn = {}  # function -> (kind, add tuple)
for (fm, fn, dm) in adds:
    md = meta.get(dm)
    if md is None: continue
    fl = flags_of(md)
    if 'v' in fl or 'f' in fl: continue
    if '2' in fl and '1' not in fl: kind = '2'
    elif '1' in fl or fl == '': kind = '1'
    else: kind = '2'
    if name_of(md) in SKIP_NAMES: continue
    modefn.setdefault(fn, (kind, fm, dm, md))

allfn = set(x[1] for x in adds)
def build(a, b, kind):
    body = []; kept = []
    for (x, y, text) in chunks(a, b):
        fn = func_name(text)
        s = text.strip()
        if s.startswith('static const char _data_'): continue          # metadata is rebuilt below
        if re.match(r'#(ifndef|endif)\s*(//)?\s*WLED_DISABLE_(2D|PARTICLE)', s): continue
        if not (fn and fn in allfn) and BAD.search(nocomment(text)) and not s.startswith('#'): continue   # helper for music or particle effects
        if fn and fn in allfn:
            info = modefn.get(fn)
            if fn == 'mode_static':
                if kind == '2': continue
                body.append(text); continue                     # needed by FX_FALLBACK_STATIC, not offered as an effect
            if not info or info[0] != kind or BAD.search(nocomment(text)): continue   # audio, other kind, or needs things we do not have
            kept.append(fn)
        body.append(text)
    return '\n'.join(body), kept

def idx(pat, start=0):
    for i in range(start, len(L)):
        if pat in L[i]: return i + 1
    raise SystemExit('not found ' + pat)

# classic (1D) region: after the helper section up to the first WLED_DISABLE_2D, 2D region: up to its #endif
s1 = idx('void mode_static(void)') - 4
e1 = idx('#ifndef WLED_DISABLE_2D', 5000) - 1
e2 = idx('#endif // WLED_DISABLE_2D', e1)
body1, k1 = build(s1, e1, '1')
body2, k2 = build(e1 + 1, e2, '2')
s3 = idx('#ifndef WLED_DISABLE_2D', e2); e3 = idx('#ifndef WLED_DISABLE_PARTICLESYSTEM2D', s3) - 1
body3, k3 = build(s3, e3, '2')
body2 = body2 + '\n' + body3; k2 = k2 + k3

HDR = '''/*
 * Effect code copied from WLED (https://github.com/wled/WLED), version v16.0.1, file wled00/FX.cpp
 *
 * Copyright (c) 2016  Harm Aldick
 * Copyright (c) 2016-present Christian Schwinne and individual WLED contributors
 * Licensed under the EUPL v. 1.2 or later  (see LICENSE in the root of this repository)
 * Adapted from code originally licensed under the MIT license. Modified heavily for WLED.
 * The effects of Andrew Tuline, Aircoookie, Yaroslav Kramar, Ewoud Wijma and the other WLED authors
 * keep their notices in the code below.
 *
 * This file is GENERATED by tools/gen_wled.py from WLED's FX.cpp. Only the music reactive effects
 * and the ones that need WLED's segment/particle/font system are left out.
 * Changes made for this project: the effect functions are unchanged; WledCore.h supplies a small
 * replacement for WLED's Segment / strip classes so that the code compiles unchanged on the pad and the stick.
 */
'''
def table(kind, kept):
    rows = []
    for fn in kept:
        kind_, fm, dm, md = modefn[fn]
        rows.append((fm, fn, md))
    return rows

a_ = idx('#define IBN 5100'); b_ = idx('static um_data_t* getAudioData()') - 1
prelude = '\n'.join(L[a_ - 1:b_]).replace('static PRNG prng(hw_random());', 'static PRNG prng(hw_random());') + '\n'
def emit(path, guard, body, kept, kind, tname):
    rows = table(kind, kept)
    o = [HDR, '#pragma once', '#include "WledCore.h"', 'namespace wfx {', '', prelude if tname == 'WFX1D' else '', body, '']
    o.append('static const WfxDef %s[] = {' % tname)
    for (fm, fn, md) in rows:
        o.append('  {"%s", &%s, "%s"},' % (name_of(md).replace('"', '\\"'), fn, md.replace('\\', '\\\\').replace('"', '\\"')))
    o.append('};')
    o.append('constexpr uint8_t %s_N = sizeof(%s) / sizeof(%s[0]);' % (tname, tname, tname))
    o.append('}  // namespace wfx')
    open(path, 'w', encoding='utf-8').write('\n'.join(o) + '\n')
    return rows

os.makedirs(out, exist_ok=True)
r1 = emit(os.path.join(out, 'WledFx1D.h'), 'X', body1, k1, '1', 'WFX1D')
r2 = emit(os.path.join(out, 'WledFx2D.h'), 'X', body2, k2, '2', 'WFX2D')
print('1D effects kept:', len(r1), ' 2D effects kept:', len(r2))
json.dump({'1D': [x[2].split('@')[0] for x in r1], '2D': [x[2].split('@')[0] for x in r2]}, open(os.path.join(out, 'names.json'), 'w'))

# ---- report what was left out
if os.environ.get('WFX_REPORT'):
    keptset = set(k1 + k2)
    for (fm, fn, dm) in adds:
        md = meta.get(dm)
        if md is None: continue
        if fn in keptset: continue
        fl = flags_of(md)
        why = 'audio' if ('v' in fl or 'f' in fl) else 'other'
        print(why, fm, fn, name_of(md), '|', fl)

# ---- assemble WledCore.h from the template and verbatim pieces of WLED
def lines_of(fn): return open(os.path.join(src, fn), encoding='utf-8').read().split('\n')
def rng(fn, a, b): return '\n'.join(lines_of(fn)[a - 1:b]) + '\n'
def find(fn, pat, start=1):
    ls = lines_of(fn)
    for i in range(start - 1, len(ls)):
        if pat in ls[i]: return i + 1
    raise SystemExit('not found: ' + pat)
def func_end(fn, a):                       # first line starting with '}' at or after a
    ls = lines_of(fn); i = a
    while not ls[i - 1].startswith('}'): i += 1
    return i
here = os.path.dirname(os.path.abspath(__file__))
T = open(os.path.join(here, 'WledCore.template.h'), encoding='utf-8').read()
a = find('prng.h', '// Simple and fast'); prng = rng('prng.h', a, find('prng.h', '};', find('prng.h', 'class PRNG')))
a = find('wled_math.cpp', '// 16-bit, integer based Bhaskara'); b = find('wled_math.cpp', '#define ATAN2_CONST_A') - 1
math = rng('wled_math.cpp', a, b)
a2 = find('wled_math.cpp', '// bit-wise integer square root'); math += rng('wled_math.cpp', a2, func_end('wled_math.cpp', a2 + 1))
math += rng('wled_math.cpp', b + 1, find('wled_math.cpp', 'float floor_t') - 1)       # atan2_t constants and function
a = find('util.cpp', '#define PERLIN_SHIFT'); b = find('util.cpp', 'uint8_t perlin8(uint16_t x, uint16_t y, uint16_t z)'); perlin = rng('util.cpp', a, func_end('util.cpp', b))
a = find('util.cpp', 'uint16_t beat88('); beat = rng('util.cpp', a - 3, find('util.cpp', '// end of FastLED functions') - 1)
a = find('colors.cpp', ' * color blend function') - 1; colors = rng('colors.cpp', a, find('colors.cpp', ' * color adjustment in HSV color space') - 2)
a = find('colors.cpp', 'WLED_O3_ATTR void adjust_color'); adjust = rng('colors.cpp', a, func_end('colors.cpp', a))
a = find('colors.cpp', 'uint32_t ColorFromPalette(const CRGBPalette16'); cfp = rng('colors.cpp', a, func_end('colors.cpp', a))
pal = rng('palettes.cpp', find('palettes.cpp', '// Cloudy color palette'), len(lines_of('palettes.cpp')))
a = find('FX_2Dfcn.cpp', 'void Segment::drawCircle('); b = find('FX_2Dfcn.cpp', 'void Segment::wu_pixel(')
draw = rng('FX_2Dfcn.cpp', a, b - 2) + rng('FX_2Dfcn.cpp', b - 1, func_end('FX_2Dfcn.cpp', b))
draw = re.sub(r'^void Segment::', '  void ', draw, flags=re.M).replace('Segment::', '')
draw = re.sub(r'^(#define WU_WEIGHT.*)$', r'\1', draw, flags=re.M)
draw += open(os.path.join(here, 'draw2d_extra.txt')).read()
fxm = '\n'.join(l for l in lines_of('FX.h') if re.match(r'#define FX_MODE_\w+\s+\d+', l)) + '\n'
for k, v in (('FXMODES', fxm), ('PRNG', prng), ('MATH', math), ('PERLIN', perlin), ('BEAT', beat), ('COLORS', colors), ('ADJUST', adjust + cfp.replace('uint32_t ColorFromPalette(const CRGBPalette16& pal, unsigned index, uint8_t brightness, TBlendType blendType) {','template <typename IDXT> uint32_t ColorFromPalette(const CRGBPalette16& pal, IDXT idx_in, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND) {\n  unsigned index = (unsigned)idx_in;')),
             ('PALETTES', pal), ('DRAW2D', draw), ('BLUR2D', open(os.path.join(here, 'blur2d.txt')).read())):
    T = T.replace('@@%s@@' % k, v)
open(os.path.join(out, 'WledCore.h'), 'w', encoding='utf-8').write(T)
print('WledCore.h written,', len(T.split('\n')), 'lines')

# ---- keep WLED's macros (R, G, B, W, MIN, MAX, RED ...) from leaking into the sketch:
# WledCore.h saves the sketch's own definitions with push_macro, WledUndef.h removes ours and restores them.
core = open(os.path.join(out, 'WledCore.h'), encoding='utf-8').read()
names = []
for m in re.finditer(r'^\s*#\s*define\s+(\w+)', core, re.M):
    if m.group(1) not in names and m.group(1) not in ('WFX_RND', 'WFX_CALLOC'): names.append(m.group(1))
push = '\n'.join('#pragma push_macro("%s")\n#undef %s' % (n, n) for n in names) + '\n'
core = core.replace('#pragma once\n', '#pragma once\n// WLED defines many short macro names; the sketch\'s own ones are saved here and restored by WledUndef.h\n' + push, 1)
open(os.path.join(out, 'WledCore.h'), 'w', encoding='utf-8').write(core)
und = ['/* Generated by tools/gen_wled.py: drops WLED\'s macros and restores the sketch\'s own. Include after WledFx*.h */', '#pragma once']
und += ['#undef %s\n#pragma pop_macro("%s")' % (n, n) for n in reversed(names)]
open(os.path.join(out, 'WledUndef.h'), 'w', encoding='utf-8').write('\n'.join(und) + '\n')
print('macros wrapped:', len(names))

# ---- names for the setup page of the pad (the stick effect list is typed into the page, so it is spliced in here)
nm = json.load(open(os.path.join(out, 'names.json')))
js = ''.join(',"W:%s"' % x.replace('\\', '\\\\').replace('"', '\\"') for x in nm['1D'])
open(os.path.join(out, 'WledNames.h'), 'w', encoding='utf-8').write(
    '/* Generated by tools/gen_wled.py. The WLED strip effects as a piece of JavaScript for the setup page */\n#pragma once\n'
    '#define WFX1D_JS_NAMES "%s"\n' % js.replace('\\', '\\\\').replace('"', '\\"'))
print('names header written')
