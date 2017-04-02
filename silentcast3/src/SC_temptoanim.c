/*
 *  Filename: SC_temptoanim.c 
 *  App Name: Silentcast <https://github.com/colinkeenan/silentcast>
 *  Copyright © 2017 Colin N Keenan <colinnkeenan@gmail.com>
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 *  
 * 
 *  Description: Convert temp.mkv or *.png created by Silentcast into
 *  an animated gif or movie according to configuration settings.
 *
 */


/*pointers defined in main.c: P("working_dir") P("area") P("p_fps") P("p_anims_from_temp") P("p_gif") P("p_pngs") P("p_webm") P("p_mp4")
 * Translating variables from old bash version of temptoanim:
 * anims_from: was defined but actually never used in bash temptoanim
 *
 * otype: "png" if *p_pngs && !(*p_gif || *p_webm || *p_mp4), "gif" if *p_gif, "webm" if *p_webm, "mp4" if *p_mp4 
 *        (I think bash version was run multiple times to get all selected outputs)
 * rm_png: !(*p_pngs)
 * use_pngs: !(*p_anims_from_temp) && (*p_webm || *p_mp4) and was also used to use exising pngs without generating them from temp.mkv
 * gen_pngs: *p_pngs || (!(*p_anims_from_temp) && (*p_webm || *p_mp4))
 * fps: *p_fps
 *
 * cut, total_cut, group, count: These were for reducing the number of pngs and adjusting the rate if user chose to do so due to memory limitation
*/
