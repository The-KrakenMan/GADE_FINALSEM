#version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 vColor;

        uniform vec3 colorin;

        out vec3 outColor;
        void main()
        {
           gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
           outColor = vColor;
           }
        