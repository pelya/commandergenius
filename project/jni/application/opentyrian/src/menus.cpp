/*
 * OpenTyrian Classic: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "config.h"
#include "episodes.h"
#include "fonthand.h"
#include "keyboard.h"
#include "menus.h"
#include "nortsong.h"
#include "opentyr.h"
#include "palette.h"
#include "picload.h"
#include "setup.h"
#include "sprite.h"
#include "video.h"

char episode_name[6][31], difficulty_name[7][21], gameplay_name[5][26];

bool
select_menuitem_by_touch(JE_byte menu_top, JE_byte menu_spacing, JE_shortint menu_item_count, JE_shortint *current_item)
{
	if (!mousedown)
		return false;

	char new_item = (mouse_y - menu_top) / menu_spacing;

	if (mouse_y >= menu_top && mouse_y < menu_top + (menu_item_count+1) * menu_spacing)
	{
		if (new_item == *current_item)
			return false;

		JE_playSampleNum(S_CURSOR);

		*current_item = new_item;
	}
	return true;
}

bool select_gameplay( void )
{
	JE_loadPic(VGAScreen, 2, false);
	JE_dString(VGAScreen, JE_fontCenter(gameplay_name[0], FONT_SHAPES), 20, gameplay_name[0], FONT_SHAPES);

	const JE_byte menu_top = 30, menu_spacing = 24;
	JE_shortint gameplay = 1,
	    gameplay_max = 4;

	bool fade_in = true;
	for (; ; )
	{
		for (int i = 1; i <= gameplay_max; i++)
		{
			JE_outTextAdjust(VGAScreen, JE_fontCenter(gameplay_name[i], SMALL_FONT_SHAPES), i * menu_spacing + menu_top, gameplay_name[i], 15, - 4 + (i == gameplay ? 2 : 0) - (i == 4 ? 4 : 0), SMALL_FONT_SHAPES, true);
		}
		JE_showVGA();

		if (fade_in)
		{
			fade_palette(colors, 10, 0, 255);
			fade_in = false;
		}

		JE_word temp = 0;
		JE_textMenuWait(&temp, false);

		if (select_menuitem_by_touch(menu_top, menu_spacing, gameplay_max, &gameplay))
			continue;

		if (newkey)
		{
			switch (lastkey_sym)
			{
			case SDLK_UP:
			case SDLK_LCTRL:
				gameplay--;
				if (gameplay < 1)
				{
					gameplay = gameplay_max;
				}
				JE_playSampleNum(S_CURSOR);
				break;
			case SDLK_DOWN:
			case SDLK_LALT:
				gameplay++;
				if (gameplay > gameplay_max)
				{
					gameplay = 1;
				}
				JE_playSampleNum(S_CURSOR);
				break;

			case SDLK_RETURN:
			case SDLK_SPACE:
				if (gameplay == 4)
				{
					JE_playSampleNum(S_SPRING);
					/* TODO: NETWORK */
					fprintf(stderr, "error: networking via menu not implemented\n");
					break;
				}
				JE_playSampleNum(S_SELECT);
				fade_black(10);

				onePlayerAction = (gameplay == 2);
				twoPlayerMode = (gameplay == 3);
				return true;

			case SDLK_ESCAPE:
				JE_playSampleNum(S_SPRING);
				/* fading handled elsewhere
				fade_black(10); */

				return false;

			default:
				break;
			}
		}
	}
}

bool select_episode( void )
{
	JE_loadPic(VGAScreen, 2, false);
	JE_dString(VGAScreen, JE_fontCenter(episode_name[0], FONT_SHAPES), 20, episode_name[0], FONT_SHAPES);

	const JE_byte menu_top = 20, menu_spacing = 30;
	JE_shortint episode = 1,
	    episode_max = EPISODE_MAX - 1;

	bool fade_in = true;
	for (; ; )
	{
		for (int i = 1; i <= episode_max; i++)
		{
			JE_outTextAdjust(VGAScreen, 20, i * menu_spacing + menu_top, episode_name[i], 15, -4 + (i == episode ? 2 : 0) - (!episodeAvail[i - 1] ? 4 : 0), SMALL_FONT_SHAPES, true);
		}
		JE_showVGA();

		if (fade_in)
		{
			fade_palette(colors, 10, 0, 255);
			fade_in = false;
		}

		JE_word temp = 0;
		JE_textMenuWait(&temp, false);

		if (select_menuitem_by_touch(menu_top, menu_spacing, episode_max, &episode))
			continue;

		if (newkey)
		{
			switch (lastkey_sym)
			{
			case SDLK_UP:
			case SDLK_LCTRL:
				episode--;
				if (episode < 1)
				{
					episode = episode_max;
				}
				JE_playSampleNum(S_CURSOR);
				break;
			case SDLK_DOWN:
			case SDLK_LALT:
				episode++;
				if (episode > episode_max)
				{
					episode = 1;
				}
				JE_playSampleNum(S_CURSOR);
				break;

			case SDLK_RETURN:
			case SDLK_SPACE:
				if (!episodeAvail[episode - 1])
				{
					JE_playSampleNum(S_SPRING);
					break;
				}
				JE_playSampleNum(S_SELECT);
				fade_black(10);

				JE_initEpisode(episode);
				initial_episode_num = episodeNum;
				return true;

			case SDLK_ESCAPE:
				JE_playSampleNum(S_SPRING);
				/* fading handled elsewhere
				fade_black(10); */

				return false;

			default:
				break;
			}
		}
	}
}

bool select_difficulty( void )
{
	JE_loadPic(VGAScreen, 2, false);
	JE_dString(VGAScreen, JE_fontCenter(difficulty_name[0], FONT_SHAPES), 20, difficulty_name[0], FONT_SHAPES);

	const JE_byte menu_top = 30, menu_spacing = 24;
	difficultyLevel = 2;
	JE_shortint difficulty_max = 3;

	bool fade_in = true;
	for (; ; )
	{
		for (int i = 1; i <= difficulty_max; i++)
		{
			JE_outTextAdjust(VGAScreen, JE_fontCenter(difficulty_name[i], SMALL_FONT_SHAPES), i * menu_spacing + menu_top, difficulty_name[i], 15, -4 + (i == difficultyLevel ? 2 : 0), SMALL_FONT_SHAPES, true);
		}
		JE_showVGA();

		if (fade_in)
		{
			fade_palette(colors, 10, 0, 255);
			fade_in = false;
		}

		JE_word temp = 0;
		JE_textMenuWait(&temp, false);

		if (select_menuitem_by_touch(menu_top, menu_spacing, difficulty_max, &difficultyLevel))
			continue;

		if (SDL_GetModState() & KMOD_SHIFT)
		{
			if ((difficulty_max < 4 && keysactive[SDLK_g]) ||
			    (difficulty_max == 4 && keysactive[SDLK_RIGHTBRACKET]))
			{
				difficulty_max++;
			}
		} else if (difficulty_max == 5 && keysactive[SDLK_l] && keysactive[SDLK_o] && keysactive[SDLK_r] && keysactive[SDLK_d]) {
			difficulty_max++;
		}

		if (newkey)
		{
			switch (lastkey_sym)
			{
			case SDLK_UP:
			case SDLK_LCTRL:
				difficultyLevel--;
				if (difficultyLevel < 1)
				{
					difficultyLevel = difficulty_max;
				}
				JE_playSampleNum(S_CURSOR);
				break;
			case SDLK_DOWN:
			case SDLK_LALT:
				difficultyLevel++;
				if (difficultyLevel > difficulty_max)
				{
					difficultyLevel = 1;
				}
				JE_playSampleNum(S_CURSOR);
				break;

			case SDLK_RETURN:
			case SDLK_SPACE:
				JE_playSampleNum(S_SELECT);
				/* fading handled elsewhere
				fade_black(10); */

				if (difficultyLevel == 6)
				{
					difficultyLevel = 8;
				} else if (difficultyLevel == 5) {
					difficultyLevel = 6;
				}
				return true;

			case SDLK_ESCAPE:
				JE_playSampleNum(S_SPRING);
				/* fading handled elsewhere
				fade_black(10); */

				return false;

			default:
				break;
			}
		}
	}
}

// kate: tab-width 4; vim: set noet:
