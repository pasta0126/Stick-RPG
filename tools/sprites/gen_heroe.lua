-- Genera la PRIMERA versión de assets/sprites/heroe.aseprite.
-- A partir de ahí el .aseprite es la fuente y se edita a mano en Aseprite;
-- este script solo sirve para regenerar desde cero (sobrescribe el archivo).
--
-- Uso: aseprite -b --script-param out=assets/sprites/heroe.aseprite \
--        --script tools/sprites/gen_heroe.lua
--
-- Convenciones (ver GDD §11):
--  * Modo indexado con la paleta global de 16 colores (src/palette.h).
--  * Índice 1 (NOCHE) = transparente. Los personajes no usan ese color.
--  * Capas de equipo dibujadas en GRIS_CLARO (4) / GRIS (3): el juego las
--    recolorea según la rareza. BLANCO (5) es brillo y no se recolorea.

local OUT = (app.params and app.params.out) or "assets/sprites/heroe.aseprite"
local W, H = 32, 36
local KEY = 1

local PAL = {
  {0x0B,0x0B,0x14},{0x1E,0x21,0x40},{0x33,0x40,0x6B},{0x5B,0x6B,0x8C},
  {0xA6,0xAE,0xBF},{0xF2,0xF2,0xEB},{0x4A,0x2E,0x22},{0x8C,0x5A,0x3C},
  {0xE8,0xB5,0x8A},{0xC2,0x3B,0x3B},{0xF0,0x8A,0x3C},{0xF7,0xD0,0x46},
  {0x4C,0xB0,0x50},{0x2A,0x6B,0x4F},{0x4A,0xA3,0xF0},{0x9A,0x5B,0xD6},
}

-- Caracteres de las piezas → índice de paleta.
local CH = { k=0, h=6, s=8, b=7, B=6, y=11, g=4, d=3, w=5, r=9 }

-- ------------------------------------------------------------------ piezas
local HEAD = {
  ".hhhhhh.",
  "hhhhhhhh",
  "hhhsssss",
  "hhsssskb",
  "hhssssss",
  ".hssssss",
  "..sssss.",
}
local TORSO = {
  "bbbbbbb",
  "bbbbbbb",
  "bbbbbbb",
  "bbbbbbb",
  "bbbbbbb",
  "BBByBBB",
  "BBBBBBB",
  "BBBBBBB",
}
local PECHERA = {
  "ggggggww",
  "gwggggdg",
  "ggggggd.",
  "gggggdd.",
  "ddddddd.",
}
local CASCO = {
  "..gggggg..",
  ".gwgggggg.",
  ".ggggggggg",
  ".dddddddd.",
  ".dd.......",
  ".dd.......",
}
local ESCUDO = {
  ".ggggg.",
  "gwggggd",
  "gggyggd",
  "ggyyygd",
  "gggyggd",
  "ggggggd",
  ".ggggd.",
  "..ggd..",
}
local BOTA = {
  "gg..",
  "ggg.",
  "dddd",
}

-- ------------------------------------------------------------------ poses
-- bx, by: desplazamiento del cuerpo. legs: {dx atrás, lift atrás, dx delante,
-- lift delante}. hand: mano delantera. dir: dirección de la espada.
local POSES = {
  -- idle
  {bx=0, by=0, legs={-1,0, 1,0}, hand={17,25}, dir={0.55,-1}},
  {bx=0, by=1, legs={-1,0, 1,0}, hand={17,26}, dir={0.55,-1}},
  -- run
  {bx=0, by=0,  legs={-3,1, 3,0}, hand={16,24}, dir={0.45,-1}},
  {bx=0, by=-1, legs={-1,0, 1,2}, hand={16,23}, dir={0.45,-1}},
  {bx=0, by=0,  legs={3,0, -3,1}, hand={16,24}, dir={0.45,-1}},
  {bx=0, by=-1, legs={1,2, -1,0}, hand={16,23}, dir={0.45,-1}},
  -- attack: preparación, tajo, remate
  {bx=-1, by=0, legs={-2,0, 2,0}, hand={11,19}, dir={-0.7,-1}},
  {bx=2,  by=0, legs={-3,0, 3,0}, hand={19,21}, dir={1,-0.05}},
  {bx=2,  by=1, legs={-3,0, 3,0}, hand={19,26}, dir={1,0.7}},
  -- hit
  {bx=-2, by=0, legs={-1,0, 1,0}, hand={14,25}, dir={0.2,-1}},
}
local TAGS = {
  {name="idle",   from=1, to=2,  ms=400},
  {name="run",    from=3, to=6,  ms=100},
  {name="attack", from=7, to=9,  ms=90},
  {name="hit",    from=10, to=10, ms=200},
}

-- ------------------------------------------------------------- utilidades
local function put(img, x, y, c)
  x, y = math.floor(x + 0.5), math.floor(y + 0.5)
  if x >= 0 and y >= 0 and x < W and y < H then img:putPixel(x, y, c) end
end

local function stamp(img, part, x, y)
  for row = 1, #part do
    local line = part[row]
    for col = 1, #line do
      local c = CH[line:sub(col, col)]
      if c then put(img, x + col - 1, y + row - 1, c) end
    end
  end
end

local function line(img, x0, y0, x1, y1, c)
  local n = math.max(math.abs(x1 - x0), math.abs(y1 - y0))
  if n < 1 then put(img, x0, y0, c) return end
  for i = 0, n do
    put(img, x0 + (x1 - x0) * i / n, y0 + (y1 - y0) * i / n, c)
  end
end

local function thick(img, x0, y0, x1, y1, c)
  line(img, x0, y0, x1, y1, c)
  if math.abs(x1 - x0) > math.abs(y1 - y0) then
    line(img, x0, y0 + 1, x1, y1 + 1, c)
  else
    line(img, x0 + 1, y0, x1 + 1, y1, c)
  end
end

-- Contorno negro alrededor de lo dibujado en la capa.
local function outline(img)
  local src = img:clone()
  for y = 0, H - 1 do
    for x = 0, W - 1 do
      if src:getPixel(x, y) == KEY then
        for _, d in ipairs({{1,0},{-1,0},{0,1},{0,-1}}) do
          local nx, ny = x + d[1], y + d[2]
          if nx >= 0 and ny >= 0 and nx < W and ny < H and
             src:getPixel(nx, ny) ~= KEY then
            img:putPixel(x, y, 0)
            break
          end
        end
      end
    end
  end
end

local function newImage()
  local img = Image(W, H, ColorMode.INDEXED)
  img:clear(KEY)
  return img
end

-- --------------------------------------------------------------- dibujo
local FOOT_Y = 31

local function legEnds(p, which)
  local hx = (which == 1) and 10 or 13
  local dx, lift = p.legs[which * 2 - 1], p.legs[which * 2]
  return hx + p.bx, 26 + p.by, hx + dx, FOOT_Y - lift
end

local DRAW = {}

DRAW.escudo = function(img, p)
  stamp(img, ESCUDO, 3 + p.bx, 19 + p.by)
end

DRAW.cuerpo = function(img, p)
  -- brazo trasero
  thick(img, 10 + p.bx, 19 + p.by, 8 + p.bx, 23 + p.by, CH.s)
  -- piernas
  for which = 1, 2 do
    local hx, hy, fx, fy = legEnds(p, which)
    thick(img, hx, hy, fx, fy, CH.B)
  end
  stamp(img, TORSO, 9 + p.bx, 18 + p.by)
  stamp(img, HEAD, 9 + p.bx, 11 + p.by)
end

DRAW.botas = function(img, p)
  for which = 1, 2 do
    local _, _, fx, fy = legEnds(p, which)
    stamp(img, BOTA, fx, fy + 1)
  end
end

DRAW.pechera = function(img, p)
  stamp(img, PECHERA, 9 + p.bx, 18 + p.by)
end

DRAW.casco = function(img, p)
  stamp(img, CASCO, 8 + p.bx, 9 + p.by)
end

DRAW.brazo = function(img, p)
  local hx, hy = p.hand[1], p.hand[2] + p.by
  local dx, dy = p.dir[1], p.dir[2]
  local len = math.sqrt(dx * dx + dy * dy)
  dx, dy = dx / len, dy / len
  local px, py = -dy, dx  -- perpendicular
  -- empuñadura
  line(img, hx - dx * 2, hy - dy * 2, hx, hy, CH.b)
  -- guarda
  local gx, gy = hx + dx, hy + dy
  line(img, gx - px * 2, gy - py * 2, gx + px * 2, gy + py * 2, CH.y)
  -- hoja (2 px: filo + brillo)
  line(img, hx + dx * 2, hy + dy * 2, hx + dx * 12, hy + dy * 12, CH.g)
  line(img, hx + dx * 2 + px, hy + dy * 2 + py,
            hx + dx * 11 + px, hy + dy * 11 + py, CH.w)
  -- brazo y mano
  thick(img, 14 + p.bx, 19 + p.by, hx, hy, CH.s)
  put(img, hx, hy, CH.s); put(img, hx + 1, hy, CH.s)
  put(img, hx, hy + 1, CH.s); put(img, hx + 1, hy + 1, CH.s)
end

-- Capas de abajo arriba.
local LAYERS = {"escudo", "cuerpo", "botas", "pechera", "casco", "brazo"}

-- --------------------------------------------------------------- sprite
local spr = Sprite(W, H, ColorMode.INDEXED)
local pal = Palette(16)
for i, c in ipairs(PAL) do
  pal:setColor(i - 1, Color{r=c[1], g=c[2], b=c[3]})
end
spr:setPalette(pal)
spr.transparentColor = KEY

for _ = 2, #POSES do spr:newEmptyFrame() end

local first = spr.layers[1]
local layers = {}
for i, name in ipairs(LAYERS) do
  local l = (i == 1) and first or spr:newLayer()
  l.name = name
  layers[i] = l
end

for f, p in ipairs(POSES) do
  for i, name in ipairs(LAYERS) do
    local img = newImage()
    DRAW[name](img, p)
    outline(img)
    spr:newCel(layers[i], f, img, Point(0, 0))
  end
end

for _, t in ipairs(TAGS) do
  local tag = spr:newTag(t.from, t.to)
  tag.name = t.name
  for f = t.from, t.to do spr.frames[f].duration = t.ms / 1000 end
end

spr:saveAs(OUT)
print("OK " .. OUT .. " " .. #spr.frames .. " frames, " .. #spr.layers .. " capas")
