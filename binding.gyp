{
  "targets": [
    {
      "target_name": "my_allocator",
      "sources": [ 
        "src/addon.cpp", 
        "src/allocator.cpp", 
        "src/arena.cpp" 
      ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "defines": [ "NAPI_DISABLE_CPP_EXCEPTIONS" ]
    }
  ]
}