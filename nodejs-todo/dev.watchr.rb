def run
  system("cls")
  system("node spec.js")
end

watch ('.*.js$') { run }
