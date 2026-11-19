-- Crea un .aseprite indexado a partir de un archivo de celdas generado por
-- tools/art/build.py.
--
-- Uso: aseprite -b --script-param src=<celdas.txt> --script-param out=<x.aseprite>
--        --script tools/sprites/assemble.lua
--
-- Formato del archivo de celdas (texto):
--   SIZE w h frames key
--   PAL r g b            (16 líneas)
--   LAYER nombre         (de abajo arriba)
--   CEL frame            (seguido de h líneas de w dígitos hex)
--   TAG nombre desde cuantos ms

local src = assert(app.params.src, "falta src")
local out = assert(app.params.out, "falta out")

local f = assert(io.open(src, "r"))
local lines = {}
for l in f:lines() do lines[#lines + 1] = l end
f:close()

local i = 1
local function nextLine() local l = lines[i]; i = i + 1; return l end

local w, h, frames, key = nextLine():match("SIZE (%d+) (%d+) (%d+) (%d+)")
w, h, frames, key = tonumber(w), tonumber(h), tonumber(frames), tonumber(key)

local spr = Sprite(w, h, ColorMode.INDEXED)
local pal = Palette(16)
for k = 0, 15 do
  local r, g, b = nextLine():match("PAL (%d+) (%d+) (%d+)")
  pal:setColor(k, Color{r=tonumber(r), g=tonumber(g), b=tonumber(b)})
end
spr:setPalette(pal)
spr.transparentColor = key
for _ = 2, frames do spr:newEmptyFrame() end

local layer = nil
local firstLayer = true
local tags = {}
while i <= #lines do
  local l = nextLine()
  local name = l:match("^LAYER (.+)$")
  if name then
    if firstLayer then layer = spr.layers[1]; firstLayer = false
    else layer = spr:newLayer() end
    layer.name = name
  end
  local fr = l:match("^CEL (%d+)$")
  if fr then
    local img = Image(w, h, ColorMode.INDEXED)
    for y = 0, h - 1 do
      local row = nextLine()
      for x = 0, w - 1 do
        img:putPixel(x, y, tonumber(row:sub(x + 1, x + 1), 16))
      end
    end
    spr:newCel(layer, tonumber(fr) + 1, img, Point(0, 0))
  end
  local tn, from, cnt, ms = l:match("^TAG (%S+) (%d+) (%d+) (%d+)$")
  if tn then tags[#tags + 1] = {tn, tonumber(from), tonumber(cnt), tonumber(ms)} end
end

for _, t in ipairs(tags) do
  local tag = spr:newTag(t[2] + 1, t[2] + t[3])
  tag.name = t[1]
  for k = t[2] + 1, t[2] + t[3] do spr.frames[k].duration = t[4] / 1000 end
end

spr:saveAs(out)
print("OK " .. out)
