# ==================================================================================================
# NOTE:
#   This file fully encapsulates AI-generated _utility_ code for dependency graph generation.
# 
# PROMPT SUMMARY (distilled):
#   Build a self-contained CMake utility that reads ENABLED_MODULES and MODULE_<name> metadata,
#   generates dep_graph.dot at configure time, and defines a dep_graph target that generates SVG
#   and interactive HTML together. Keep it non-fatal when graphviz dot is missing. Group modules
#   by top-level folder, reduce transitive redundant edges for clarity, avoid hardcoded colors by
#   module names, and support click-to-highlight dependency subgraphs in HTML.

# ==================================================================================================
# Function: generate_dependency_graph
#
# Description:
#   Generates a Graphviz DOT file for the enabled module dependency graph and registers a
#   'dep_graph' target to render it to SVG using the 'dot' tool.
#
#   - Modules are clustered by their top-level folder under modules/ (e.g. common, probe, rpi).
#   - Arrows point from a dependent module toward the module it depends on.
#   - Direct edges are reduced transitively where possible to avoid visual clutter.
#   - A companion HTML viewer is generated with click-to-highlight for per-module dependency
#     subgraphs.
#   - Must be called after enumerate_modules() so that ENABLED_MODULES and MODULE_*_PATH /
#     MODULE_*_DEPENDS_ON cache variables are populated.
#
# Parameters:
#   OUTPUT_DIR (optional): Directory to write dep_graph.dot, dep_graph.svg and dep_graph.html.
#                          Defaults to <binary_dir>/docs.
#
function(generate_dependency_graph)
  cmake_parse_arguments(PARSE_ARGV 0 _ARG "" "OUTPUT_DIR" "")

  if(_ARG_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "generate_dependency_graph: unparsed arguments: ${_ARG_UNPARSED_ARGUMENTS}")
  endif()

  if(NOT _ARG_OUTPUT_DIR)
    set(_ARG_OUTPUT_DIR "${CMAKE_BINARY_DIR}/docs")
  endif()

  get_property(_modules GLOBAL PROPERTY ENABLED_MODULES)
  if(NOT _modules)
    add_custom_target(dep_graph
      COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --yellow
              "dep_graph skipped: ENABLED_MODULES is empty"
      COMMENT "Skipping dependency graph rendering"
      VERBATIM)
    message(STATUS "generate_dependency_graph: ENABLED_MODULES is empty - dep_graph target is a no-op")
    return()
  endif()

  # ------------------------------------------------------------------------------------------------
  # Derive group (top-level folder under modules/) for every enabled module
  set(_groups "")
  foreach(module IN LISTS _modules)
    cmake_path(RELATIVE_PATH MODULE_${module}_PATH
               BASE_DIRECTORY "${CMAKE_SOURCE_DIR}/modules"
               OUTPUT_VARIABLE _rel)
    string(FIND "${_rel}" "/" _slash_pos)
    if(_slash_pos GREATER_EQUAL 0)
      string(SUBSTRING "${_rel}" 0 ${_slash_pos} _group)
    else()
      set(_group "${_rel}")
    endif()
    set(_module_group_${module} "${_group}")
    list(APPEND _groups "${_group}")
  endforeach()
  list(REMOVE_DUPLICATES _groups)
  list(SORT _groups)

  # ------------------------------------------------------------------------------------------------
  # Colour palette - indexed slots: node-fill  node-border  cluster-bg  cluster-border  font-colour
  # Groups are assigned a slot by cycling through in sorted order.
  set(_pal_0 "#aecde3" "#2a7ab0" "#e8f3fa" "#2a7ab0" "#153d58")  # blue
  set(_pal_1 "#a8dfd4" "#1f9180" "#e6f7f5" "#1f9180" "#104840")  # teal
  set(_pal_2 "#f5d49a" "#c07820" "#fdf3e0" "#c07820" "#5a3800")  # amber
  set(_pal_3 "#cfb3e8" "#7030b8" "#f3edfb" "#7030b8" "#3a1060")  # purple
  set(_pal_4 "#b8dea8" "#3a8c20" "#ecf7e8" "#3a8c20" "#1a4a0c")  # green
  set(_pal_5 "#f0b8b8" "#b83020" "#fdecea" "#b83020" "#601010")  # red
  set(_num_palette_slots 6)

  set(_group_idx 0)
  foreach(group IN LISTS _groups)
    math(EXPR _slot "${_group_idx} % ${_num_palette_slots}")
    set(_group_pal_${group} "${_pal_${_slot}}")
    math(EXPR _group_idx "${_group_idx} + 1")
  endforeach()

  # ------------------------------------------------------------------------------------------------
  # Build direct edge list for enabled modules (src -> dep)
  set(_direct_edges "")
  foreach(module IN LISTS _modules)
    foreach(dep IN LISTS MODULE_${module}_DEPENDS_ON)
      if(dep IN_LIST _modules)
        list(APPEND _direct_edges "${module}|${dep}")
      endif()
    endforeach()
  endforeach()

  # Transitive edge reduction: remove A->C if A->...->C already exists through other nodes.
  list(LENGTH _modules _num_modules)
  math(EXPR _last_module_idx "${_num_modules} - 1")

  # Reachability matrix seeded with direct edges.
  foreach(_e IN LISTS _direct_edges)
    string(REPLACE "|" ";" _parts "${_e}")
    list(GET _parts 0 _src)
    list(GET _parts 1 _dst)
    list(FIND _modules "${_src}" _i)
    list(FIND _modules "${_dst}" _j)
    if(NOT _i EQUAL -1 AND NOT _j EQUAL -1)
      set(_reach_${_i}_${_j} ON)
    endif()
  endforeach()

  # Floyd-Warshall transitive closure on reachability.
  foreach(k RANGE ${_last_module_idx})
    foreach(i RANGE ${_last_module_idx})
      if(DEFINED _reach_${i}_${k})
        foreach(j RANGE ${_last_module_idx})
          if(DEFINED _reach_${k}_${j})
            set(_reach_${i}_${j} ON)
          endif()
        endforeach()
      endif()
    endforeach()
  endforeach()

  # Keep only non-redundant edges.
  set(_render_edges "")
  foreach(_e IN LISTS _direct_edges)
    string(REPLACE "|" ";" _parts "${_e}")
    list(GET _parts 0 _src)
    list(GET _parts 1 _dst)
    list(FIND _modules "${_src}" _i)
    list(FIND _modules "${_dst}" _j)

    set(_redundant OFF)
    foreach(k RANGE ${_last_module_idx})
      if((NOT k EQUAL _i) AND (NOT k EQUAL _j))
        if(DEFINED _reach_${_i}_${k} AND DEFINED _reach_${k}_${_j})
          set(_redundant ON)
          break()
        endif()
      endif()
    endforeach()

    if(NOT _redundant)
      list(APPEND _render_edges "${_src}|${_dst}")
    endif()
  endforeach()

  # ------------------------------------------------------------------------------------------------
  # Build DOT source
  set(_dot "digraph ${CMAKE_PROJECT_NAME}_modules {\n")
  string(APPEND _dot "  // Generated by generate_dependency_graph() - do not edit by hand\n\n")
  string(APPEND _dot "  graph [\n")
  string(APPEND _dot "    rankdir     = \"TB\"\n")
  string(APPEND _dot "    bgcolor     = \"#ffffff\"\n")
  string(APPEND _dot "    fontname    = \"Helvetica Neue, Helvetica, Arial, sans-serif\"\n")
  string(APPEND _dot "    fontsize    = 15\n")
  string(APPEND _dot "    fontcolor   = \"#222222\"\n")
  string(APPEND _dot "    label       = \"Module Dependency Graph\"\n")
  string(APPEND _dot "    labelloc    = t\n")
  string(APPEND _dot "    labeljust   = c\n")
  string(APPEND _dot "    pad         = 0.8\n")
  string(APPEND _dot "    splines     = curved\n")
  string(APPEND _dot "    nodesep     = 0.55\n")
  string(APPEND _dot "    ranksep     = 1.1\n")
  string(APPEND _dot "    concentrate = true\n")
  string(APPEND _dot "  ]\n\n")
  string(APPEND _dot "  node [\n")
  string(APPEND _dot "    shape    = box\n")
  string(APPEND _dot "    style    = \"filled,rounded\"\n")
  string(APPEND _dot "    fontname = \"Helvetica Neue, Helvetica, Arial, sans-serif\"\n")
  string(APPEND _dot "    fontsize = 11\n")
  string(APPEND _dot "    margin   = \"0.22, 0.11\"\n")
  string(APPEND _dot "    penwidth = 1.5\n")
  string(APPEND _dot "  ]\n\n")
  string(APPEND _dot "  edge [\n")
  string(APPEND _dot "    arrowhead = vee\n")
  string(APPEND _dot "    arrowsize = 0.75\n")
  string(APPEND _dot "    penwidth  = 1.2\n")
  string(APPEND _dot "    color     = \"#607080\"\n")
  string(APPEND _dot "    style     = solid\n")
  string(APPEND _dot "  ]\n\n")

  # Cluster subgraph per group
  foreach(group IN LISTS _groups)
    set(_pal "${_group_pal_${group}}")
    list(GET _pal 0 _fill)
    list(GET _pal 1 _border)
    list(GET _pal 2 _bg)
    list(GET _pal 3 _cborder)
    list(GET _pal 4 _font)

    string(APPEND _dot "  subgraph cluster_${group} {\n")
    string(APPEND _dot "    label     = \"${group}\"\n")
    string(APPEND _dot "    style     = \"filled, rounded\"\n")
    string(APPEND _dot "    fillcolor = \"${_bg}\"\n")
    string(APPEND _dot "    color     = \"${_cborder}\"\n")
    string(APPEND _dot "    penwidth  = 2\n")
    string(APPEND _dot "    fontname  = \"Helvetica Neue, Helvetica, Arial, sans-serif\"\n")
    string(APPEND _dot "    fontsize  = 12\n")
    string(APPEND _dot "    fontcolor = \"${_font}\"\n")
    string(APPEND _dot "    margin    = 14\n")
    string(APPEND _dot "    node [ fillcolor = \"${_fill}\" color = \"${_border}\" fontcolor = \"${_font}\" ]\n")
    foreach(module IN LISTS _modules)
      if(_module_group_${module} STREQUAL group)
        string(APPEND _dot "    \"${module}\"\n")
      endif()
    endforeach()
    string(APPEND _dot "  }\n\n")
  endforeach()

  # Dependency edges (A -> B: A depends on B); colour matches the source group
  string(APPEND _dot "  // Dependency edges: arrow tip points at the dependency\n")
  foreach(_e IN LISTS _render_edges)
    string(REPLACE "|" ";" _parts "${_e}")
    list(GET _parts 0 module)
    list(GET _parts 1 dep)

    set(_pal "${_group_pal_${_module_group_${module}}}")
    list(GET _pal 1 _edge_color)
    string(APPEND _dot "  \"${module}\" -> \"${dep}\" [ color = \"${_edge_color}\" ]\n")
  endforeach()

  string(APPEND _dot "}\n")

  # ------------------------------------------------------------------------------------------------
  # Write DOT at configure time
  set(_dot_file "${_ARG_OUTPUT_DIR}/dep_graph.dot")
  set(_svg_file "${_ARG_OUTPUT_DIR}/dep_graph.svg")
  set(_html_file "${_ARG_OUTPUT_DIR}/dep_graph.html")
  set(_html_script "${_ARG_OUTPUT_DIR}/render_dep_graph_html.cmake")
  file(MAKE_DIRECTORY "${_ARG_OUTPUT_DIR}")
  file(WRITE "${_dot_file}" "${_dot}")
  message(STATUS "Wrote dependency graph DOT: ${_dot_file}")

  # HTML is generated by dep_graph target (after SVG is rendered) so it always has fresh data.
  set(_html_script_content [=[
set(_html_file "@HTML_FILE@")
set(_svg_file "@SVG_FILE@")
file(READ "${_svg_file}" _svg_content)
set(_html [==[
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<title>Dependency Graph</title>
<style>
body{margin:0;font:14px/1.45 -apple-system,BlinkMacSystemFont,Segoe UI,Roboto,Helvetica,Arial,sans-serif;background:#fbfcfe;color:#1f2937;}
header{position:sticky;top:0;z-index:2;display:flex;gap:12px;align-items:center;padding:10px 14px;border-bottom:1px solid #d8e0ea;background:#ffffffcc;backdrop-filter:blur(6px);}
button{border:1px solid #b8c4d3;background:#fff;padding:6px 10px;border-radius:8px;cursor:pointer;}
#hint{opacity:.75;}
#graph{padding:10px;}
svg{max-width:100%;height:auto;border:1px solid #e4e9f0;border-radius:10px;background:#fff;}
g.node{cursor:pointer;transition:opacity .15s ease;}
g.edge{transition:opacity .15s ease;}
g.node.active polygon,g.node.active ellipse,g.node.active path{stroke:#111827 !important;stroke-width:3px !important;}
g.node.dim,g.edge.dim{opacity:.12;}
</style>
</head>
<body>
<header><button id="reset">Reset Highlight</button><div id="hint">Click a module node to highlight its dependency subgraph.</div></header>
<div id="graph">__SVG_CONTENT__</div>
<script>
const graphDiv=document.getElementById('graph');
const resetBtn=document.getElementById('reset');

function closureFrom(root, depMap){
  const seen=new Set();
  const stack=[root];
  while(stack.length){
    const n=stack.pop();
    if(seen.has(n)) continue;
    seen.add(n);
    for(const nxt of (depMap.get(n)||[])) stack.push(nxt);
  }
  return seen;
}

function applyHighlight(root,nodeByName,edgeEls,depMap){
  const keep=closureFrom(root, depMap);
  for(const [name,g] of nodeByName){
    g.classList.toggle('dim',!keep.has(name));
    g.classList.toggle('active',name===root);
  }
  for(const e of edgeEls){
    const src=e.dataset.src;
    const dst=e.dataset.dst;
    const on=keep.has(src)&&keep.has(dst);
    e.classList.toggle('dim',!on);
  }
}

function clearHighlight(nodeByName,edgeEls){
  for(const g of nodeByName.values()) g.classList.remove('dim','active');
  for(const e of edgeEls) e.classList.remove('dim');
}

const svg=graphDiv.querySelector('svg');
if(!svg){
  graphDiv.innerHTML='<p>Failed to parse embedded SVG.</p>';
}else{
  const nodeByName=new Map();
  svg.querySelectorAll('g.node').forEach(g=>{
    const t=g.querySelector('title');
    if(!t) return;
    nodeByName.set(t.textContent.trim(), g);
  });

  const edgeEls=[];
  const depMap=new Map();
  for(const name of nodeByName.keys()) depMap.set(name, []);

  svg.querySelectorAll('g.edge').forEach(g=>{
    const t=g.querySelector('title');
    if(!t) return;
    const parts=t.textContent.split('->');
    if(parts.length!==2) return;
    const src=parts[0].trim();
    const dst=parts[1].trim();
    g.dataset.src=src;
    g.dataset.dst=dst;
    edgeEls.push(g);
    if(depMap.has(src)) depMap.get(src).push(dst);
  });

  for(const [name,g] of nodeByName){
    g.addEventListener('click',ev=>{
      ev.stopPropagation();
      applyHighlight(name,nodeByName,edgeEls,depMap);
    });
  }

  resetBtn.addEventListener('click',()=>clearHighlight(nodeByName,edgeEls));
  svg.addEventListener('click',()=>clearHighlight(nodeByName,edgeEls));
}
</script>
</body>
</html>
]==])

string(REPLACE "__SVG_CONTENT__" "${_svg_content}" _html "${_html}")

file(WRITE "${_html_file}" "${_html}")
]=])
string(REPLACE "@HTML_FILE@" "${_html_file}" _html_script_content "${_html_script_content}")
string(REPLACE "@SVG_FILE@" "${_svg_file}" _html_script_content "${_html_script_content}")
file(WRITE "${_html_script}" "${_html_script_content}")

  # Register target to render SVG from DOT.
  find_program(GRAPHVIZ_DOT_EXECUTABLE dot NO_CACHE)
  if(GRAPHVIZ_DOT_EXECUTABLE)
    add_custom_target(dep_graph
      COMMAND "${GRAPHVIZ_DOT_EXECUTABLE}" -Tsvg -o "${_svg_file}" "${_dot_file}"
      COMMAND ${CMAKE_COMMAND} -P "${_html_script}"
      BYPRODUCTS "${_svg_file}" "${_html_file}"
      WORKING_DIRECTORY "${_ARG_OUTPUT_DIR}"
      COMMENT "Rendering dependency graph assets: ${_svg_file}, ${_html_file}"
      VERBATIM)
    message(STATUS "To build dependency graph, call: cmake --build ${CMAKE_BINARY_DIR} --target dep_graph")
  else()
    add_custom_target(dep_graph
      COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --yellow
              "dep_graph skipped: 'dot' executable (graphviz) not found"
      COMMENT "Skipping dependency graph rendering (graphviz not found)"
      VERBATIM)
    message(STATUS "generate_dependency_graph: 'dot' (graphviz) not found - dep_graph target is a no-op")
  endif()
endfunction()
