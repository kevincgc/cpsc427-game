﻿// internal
#include "render_system.hpp"
#include "world_system.hpp"
#include <iostream>



void RenderSystem::drawTexturedMesh(entt::entity entity,
									const mat3 &projection)
{
	static bool reflected = false;
	Motion &motion = registry.get<Motion>(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position - vec2(WorldSystem::camera.x, WorldSystem::camera.y));
	transform.rotate(motion.angle);
	transform.scale(motion.scale);

	assert(registry.view<RenderRequest>().contains(entity));

	RenderRequest &render_request = registry.get<RenderRequest>(entity);

	if (motion.velocity.x < 0) {
		render_request.is_reflected = true;
	} else if (motion.velocity.x > 0) {
		render_request.is_reflected = false;
	} // if zero keep last

	if (render_request.is_reflected) transform.reflect();

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position
		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.view<RenderRequest>().contains(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.get<RenderRequest>(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::PEBBLE)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::MINOTAUR)
	{

		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_uv_loc = glGetAttribLocation(program, "in_uv");


		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_uv_loc);
		glVertexAttribPointer(
			in_uv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		float time_total = (float)(glfwGetTime()) - game_start_time;
		GLuint time_uloc = glGetUniformLocation(program, "time");
		glUniform1f(time_uloc, time_total);

		bool flash = registry.view<Flash>().contains(entity);
		GLuint flash_uloc = glGetUniformLocation(program, "flash");
		glUniform1f(flash_uloc, flash);

		// pass gesture to the shader
		GLuint motion_uloc = glGetUniformLocation(program, "gesture");
		int player_gesture = 0;

		Motion& player_motion = registry.get<Motion>(entity);
		if (registry.view<DeathTimer>().contains(entity)) {
			player_gesture = 9;
		}
		else if (registry.view<Attack>().contains(entity)) {
			player_gesture = 3;
		}
		else if (player_motion.velocity.x == 0 && player_motion.velocity.y == 0) {
			player_gesture = 0;
		}
		else {
			player_gesture = 1;
		}

		glUniform1i(motion_uloc, player_gesture);



		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();
		assert(registry.view<RenderRequest>().contains(entity));
		GLuint texture_id_minotaur =
			texture_gl_handles[(GLuint)registry.get<RenderRequest>(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id_minotaur);
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.view<Colour>().contains(entity) ? registry.get<Colour>(entity).colour : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::drawTile(const vec2 map_coords, const MapTile map_tile, const mat3 &projection, vec2 screen)
{
	// define which texture to draw
	TEXTURE_ASSET_ID texture_asset;
	switch (map_tile) {
		case MapTile::BREAKABLE_WALL:
		case MapTile::UNBREAKABLE_WALL:
			texture_asset = TEXTURE_ASSET_ID::WALL;
			break;

		case MapTile::FREE_SPACE:
		case MapTile::ENTRANCE:
		case MapTile::EXIT:
			texture_asset = TEXTURE_ASSET_ID::FREESPACE;
			break;
		default:
			return; // don't render anything
	}

	Transform transform;

	// transform map coords to position
	vec2 position = {WorldSystem::map_coords_to_position(map_coords)};

	transform.translate(position - vec2(WorldSystem::camera.x, WorldSystem::camera.y) + vec2(map_scale.x/2.0, map_scale.y/2.0));
	transform.scale({-map_scale.x, map_scale.y});

	position = position - vec2(WorldSystem::camera.x, WorldSystem::camera.y) + vec2(map_scale.x / 2.0, map_scale.y / 2.0);
	if (position.x < -map_scale.x || position.x > screen.x + map_scale.x || position.y < -map_scale.y || position.y > screen.y + map_scale.y) {
		return; // ignore, out of screen
	}

	const RenderRequest render_request = {
		texture_asset,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE
	};

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	GLint in_position_loc = glGetAttribLocation(program, "in_position");
	GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
	gl_has_errors();
	assert(in_texcoord_loc >= 0);

	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							sizeof(TexturedVertex), (void *)0);
	gl_has_errors();

	glEnableVertexAttribArray(in_texcoord_loc);
	glVertexAttribPointer(
		in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
		(void *)sizeof(
			vec3)); // note the stride to skip the preceeding vertex position
	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	gl_has_errors();

	GLuint texture_id = texture_gl_handles[(GLuint) render_request.used_texture];

	glBindTexture(GL_TEXTURE_2D, texture_id);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}


/* tutorial reference :
/ https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01 */
void RenderSystem::drawText(const char* text, vec2 position, vec2 scale, const mat3& projection)
{
	if (strlen(text) <= 0) return; // skip, nothing drawn

	// prepare for transformation
	Transform transform;

	// glyph's slot
	FT_GlyphSlot g = face->glyph;

	// setting text's pixel size
	pixel_size = 20.f;
	FT_Set_Pixel_Sizes(face, pixel_size, pixel_size);


	// get program shader
	const GLuint program = (GLuint) effects[(int) EFFECT_ASSET_ID::TEXT];

	//single texture object to render all the glyphs
	GLuint tex;
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	gl_has_errors();

	// no aligment restriction
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//setting vertex buffer for text
	GLuint text_vbo;
	glGenBuffers(1, &text_vbo);
	GLint attribute_coord = glGetAttribLocation(program, "vertex");

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	GLuint uniform_color = glGetUniformLocation(program, "textColor");
	glUniform3f(uniform_color, 0.6 , 0.5 , 0.5);

	glEnableVertexAttribArray(attribute_coord);
	glBindBuffer(GL_ARRAY_BUFFER, text_vbo);
	glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
	gl_has_errors();
	assert(attribute_coord >= 0);



	//prevent certain artifacts when a character is not rendered exactly on pixel boundaries
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	gl_has_errors();

	for (const char* p = text; *p; p++)
	{
		if (FT_Load_Char(face, *p, FT_LOAD_RENDER)) {continue;}

		glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_RED,
				g->bitmap.width,
				g->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				g->bitmap.buffer
		);


		float x2 = position.x + g->bitmap_left * scale.x;
		float y2 = -position.y - g->bitmap_top * scale.y;
		float w = g->bitmap.width * scale.x;
		float h = g->bitmap.rows * scale.y;

		GLfloat vertices[4][4] = {
				{x2,     -y2    , 0, 0},
				{x2 + w, -y2    , 1, 0},
				{x2,     -y2 - h, 0, 1},
				{x2 + w, -y2 - h, 1, 1},
		};

		// transform vertices location
		GLuint transform_loc = glGetUniformLocation(program, "transform");
		glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
		gl_has_errors();
		GLuint projection_loc = glGetUniformLocation(program, "projection");
		glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
		gl_has_errors();

		// draw characters on screen;
		glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_DYNAMIC_DRAW);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		gl_has_errors();

		// Advance the cursor to the start of the next character
		position.x += (g->advance.x >> 6) * scale.x;
		position.y += (g->advance.y >> 6) * scale.y;
		gl_has_errors();
	}

	// clear memory
	glDeleteTextures(1, &tex);
	glDeleteBuffers(1, &text_vbo);
}









// draw the intermediate texture to the screen, with some distortion to simulate
// water
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the water texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WATER]);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint water_program = effects[(GLuint)EFFECT_ASSET_ID::WATER];
	// Set clock
	float time_total = (float)glfwGetTime() - game_start_time;
	GLuint time_uloc = glGetUniformLocation(water_program, "time");
	glUniform1f(time_uloc, time_total);
	GLuint init_game_uloc = glGetUniformLocation(water_program, "initial_game");
	glUniform1f(init_game_uloc, initial_game);
	GLuint end_game_uloc = glGetUniformLocation(water_program, "endGame");
	glUniform1f(end_game_uloc, registry.view<EndGame>().size() != 0);
	gl_has_errors();
	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(water_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();

}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(1, 1, 1, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();

	std::vector<std::vector<MapTile>> map_tiles = game_state.level.map_tiles;
	for (int i = 0; i < map_tiles.size(); i++) {
		for (int j = 0; j < map_tiles[i].size(); j++) {
			drawTile({ j, i }, map_tiles[i][j], projection_2D, vec2(w, h));
		}
	}

	// Draw all textured meshes that have a position and size component
	for (entt::entity entity : registry.view<RenderRequest>())
	{
		if (!registry.view<Motion>().contains(entity))
			continue;
		// Note, its not very efficient to access elements indirectly via the entity
		// albeit iterating through all Sprites in sequence. A good point to optimize

		// Render the sprites first
		if (!registry.view<Cutscene>().contains(entity)) {
			drawTexturedMesh(entity, projection_2D);
		}

	}

	// Render the cutscene images last so they'll be on top of the sprites
	for (entt::entity entity : registry.view <Cutscene>()) {
		if (registry.view<RenderRequest>().contains(entity)) {
			drawTexturedMesh(entity, projection_2D);
		}
	}

	// render help text
	char* renderedText_1;
	char* renderedText_2;
	if (tips.in_help_mode)
	{
		renderedText_1 = "Movement keys: arrows/W(up)/A(left)/S(down)/D(right)";
		renderedText_2 = "Click to find path.";
	}
	else
	{
		renderedText_1 = "";
		renderedText_2 = "";
	}

	vec2 text1_pos = { 1 / 2*w + (10.f * global_scaling_vector.x) * pixel_size, 60.f * global_scaling_vector.y };
	vec2 text2_pos = { 1 / 2*w + (25.f * global_scaling_vector.x) * pixel_size, 60.f * (global_scaling_vector.y) + (2.f * global_scaling_vector.y) * pixel_size };

	drawText(renderedText_1, text1_pos, { 2.f* global_scaling_vector.x, -2.5f* global_scaling_vector.y }, projection_2D);
	drawText(renderedText_2, text2_pos , { 2.f * global_scaling_vector.x, -2.5f * global_scaling_vector.y }, projection_2D);

	// Truely render to the screen
	drawToScreen();

	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	gl_has_errors();
	float right = (float)w / screen_scale;
	float bottom = (float)h / screen_scale;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}


mat3 RenderSystem::createProjectionMatrixforText()
{
	float left = 0.f;
	float top = 0.f;

	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	gl_has_errors();
	float right = (float)w / screen_scale;
	float bottom = (float)h / screen_scale;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return { {-sx, 0.f, 0.f}, {0.f, -sy, 0.f}, {tx, ty, 1.f} };
}
