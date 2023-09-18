#version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 vColor;

        uniform vec3 colorin;
        uniform mat4 transform;
        out vec3 outColor;
        void main()
        {
           gl_Position = transform*vec4(aPos, 1.0);
           outColor = vColor;
           }
        