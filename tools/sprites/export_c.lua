-- Exporta un .aseprite indexado a una cabecera C++ con datos de 4 bpp.
--
-- Uso: aseprite -b <sprite.aseprite> --script-param out=<salida.h> \
--        --script-param name=<nombre> --script tools/sprites/export_c.lua
--
-- Formato: por cada capa (de abajo arriba) y cada frame, W*H/2 bytes; cada
-- byte guarda dos píxeles (nibble alto = izquierda). Los valores son índices
-- de la paleta global; el índice transparente del sprite se conserva como
-- clave (sheet.key). Las etiquetas (tags) se exportan como animaciones.

local spr = app.activeSprite
assert(spr, "no hay sprite abierto")
assert(spr.colorMode == ColorMode.INDEXED, "el sprite debe estar en modo indexado")
local out = assert(app.params.out, "falta --script-param out=")
local name = assert(app.params.name, "falta --script-param name=")
local W, H = spr.width, spr.height
assert(W % 2 == 0, "el ancho debe ser par")
local key = spr.transparentColor

local function ident(s)
  return (s:upper():gsub("[^%w]", "_"))
end

local lines = {}
local function emit(s) lines[#lines + 1] = s end

emit("#pragma once")
emit("// Generado por tools/sprites/export_c.lua a partir de " ..
     app.fs.fileName(spr.filename) .. ". No editar a mano: `make sprites`.")
emit("")
emit("#include \"sprite.h\"")
emit("")
emit("namespace gfx {")
emit("namespace " .. name .. " {")
emit("")

local layerNames = {}
for i, l in ipairs(spr.layers) do layerNames[i] = ident(l.name) end
emit("enum Layer : uint8_t { " .. table.concat(layerNames, ", ") .. ", LAYER_COUNT };")
emit("")
for _, t in ipairs(spr.tags) do
  local ms = math.floor(spr.frames[t.fromFrame.frameNumber].duration * 1000 + 0.5)
  emit(string.format("constexpr Anim %s{%d, %d, %d};", ident(t.name),
       t.fromFrame.frameNumber - 1, t.frames, ms))
end
emit("")
emit(string.format("constexpr uint8_t kData[%d * %d * %d] = {",
     #spr.layers, #spr.frames, W * H // 2))

local usedKeyColor = false
for li, layer in ipairs(spr.layers) do
  for fi = 1, #spr.frames do
    local img = Image(W, H, ColorMode.INDEXED)
    img:clear(key)
    local cel = layer:cel(fi)
    if cel then img:drawImage(cel.image, cel.position) end
    emit(string.format("  // %s, frame %d", layer.name, fi - 1))
    local row = {}
    for y = 0, H - 1 do
      for x = 0, W - 1, 2 do
        local a, b = img:getPixel(x, y), img:getPixel(x + 1, y)
        row[#row + 1] = string.format("0x%02X", (a << 4) | b)
      end
      emit("  " .. table.concat(row, ",") .. ",")
      row = {}
    end
  end
end
emit("};")
emit("")
emit(string.format(
  "constexpr Sheet sheet{%d, %d, %d, %d, %d, kData};", W, H, #spr.frames,
  #spr.layers, key))
emit("")
emit("}  // namespace " .. name)
emit("}  // namespace gfx")

local f = assert(io.open(out, "w"))
f:write(table.concat(lines, "\n"), "\n")
f:close()
print(string.format("OK %s: %dx%d, %d frames, %d capas, %d bytes", out, W, H,
      #spr.frames, #spr.layers, #spr.layers * #spr.frames * W * H // 2))
