-- We resolve packages like OSP so that the language server can understand requires
function OnSetText(uri, text)
  local changes = {}
  -- find all dofile(string)
  -- We replace / by . and dofile / loadfile by require so sumneko understands the code
  for dofile_pos, begin_pos, pkg, name, end_pos in text:gmatch '()dofile%(()%"([%a_%-%./]*):?([%a_%.%-/]*)%"()%)' do 
    --argage.lol()
    if name == "" then 
      local last = ""
      for pkg in uri:gmatch '.-res/([%a_%-]*)/.*' do
        last = pkg
        break
      end
      -- only name, resolve uri
      name = pkg
      pkg = last
    end

    name = name:gsub("/", ".")
    name = name:gsub(".lua", "")
    local final_str = '"' .. pkg .. "." .. name .. '"'

    -- Replace path
    changes[#changes + 1] = {
      start = begin_pos,
      finish = end_pos - 1,
      text = final_str
    }

    -- Replace dofile by require
    changes[#changes + 1] = {
      start = dofile_pos,
      finish = dofile_pos + 5,
      text = "require"
    }
  end

  return changes
end
