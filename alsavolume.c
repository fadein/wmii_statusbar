/* gcc simple.c -lasound */
/*
 *   ALSA command line mixer utility
 *   Copyright (c) 1999-2000 by Jaroslav Kysela <perex@perex.cz>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

#include <ctype.h>
#include <math.h>
#include <alsa/asoundlib.h>

static int smixer_level = 0;
static struct snd_mixer_selem_regopt smixer_options;
static char card[64] = "default";

static void error(const char *fmt,...)
{
	va_list va;

	va_start(va, fmt);
	fprintf(stderr, "amixer: ");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "\n");
	va_end(va);
}

#define check_range(val, min, max) \
	(no_check ? (val) : ((val < min) ? (min) : (val > max) ? (max) : (val))) 
/* Fuction to convert from volume to percentage. val = volume */

static int convert_prange(int val, int min, int max)
{
	int range = max - min;
	int tmp;

	if (range == 0)
		return 0;
	val -= min;
	tmp = rint((double)val/(double)range * 100);
	return tmp;
}

/* Function to convert from percentage to volume. val = percentage */

#define convert_prange1(val, min, max) \
	ceil((val) * ((max) - (min)) * 0.01 + (min))

static char *get_percent(int val, int min, int max)
{
	static char str[32];
	int p;
	
	p = convert_prange(val, min, max);
	sprintf(str, "%i%%", p);
	return str;
}

enum { VOL_RAW, VOL_DB };


static char* get_selem(snd_mixer_t *handle, snd_mixer_selem_id_t *id)
{
	long pmin = 0, pmax = 0;
	long pvol;
	int psw = 1;
	snd_mixer_elem_t *elem;

	elem = snd_mixer_find_selem(handle, id);
	if (!elem) {
		error("Mixer %s simple element not found", card);
		return NULL;
	}

	if (snd_mixer_selem_has_playback_switch(elem)) {
		snd_mixer_selem_get_playback_switch(elem, SND_MIXER_SCHN_MONO, &psw);
		if (0 == psw) {
			return "Mute";
		}
	}


	if (snd_mixer_selem_has_playback_volume(elem)) {
		snd_mixer_selem_get_playback_volume_range(elem, &pmin, &pmax);
	}

	snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_MONO, &pvol);

	return get_percent(pvol, pmin, pmax);
}

static int parse_simple_id(const char *str, snd_mixer_selem_id_t *sid)
{
	int c, size;
	char buf[128];
	char *ptr = buf;

	while (*str == ' ' || *str == '\t')
		str++;
	if (!(*str))
		return -EINVAL;
	size = 1;	/* for '\0' */
	if (*str != '"' && *str != '\'') {
		while (*str && *str != ',') {
			if (size < (int)sizeof(buf)) {
				*ptr++ = *str;
				size++;
			}
			str++;
		}
	} else {
		c = *str++;
		while (*str && *str != c) {
			if (size < (int)sizeof(buf)) {
				*ptr++ = *str;
				size++;
			}
			str++;
		}
		if (*str == c)
			str++;
	}
	if (*str == '\0') {
		snd_mixer_selem_id_set_index(sid, 0);
		*ptr = 0;
		goto _set;
	}
	if (*str != ',')
		return -EINVAL;
	*ptr = 0;	/* terminate the string */
	str++;
	if (!isdigit(*str))
		return -EINVAL;
	snd_mixer_selem_id_set_index(sid, atoi(str));
       _set:
	snd_mixer_selem_id_set_name(sid, buf);
	return 0;
}

static char* sset(char *argv[])
{
	int err = 0;
	static snd_mixer_t *handle = NULL;
	char* retval;

	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	snd_mixer_selem_id_alloca(&sid);

	if (parse_simple_id(argv[0], sid)) {
		fprintf(stderr, "Wrong scontrol identifier: %s\n", argv[0]);
		return NULL;
	}

	if ((err = snd_mixer_open(&handle, 0)) < 0) {
		error("Mixer %s open error: %s\n", card, snd_strerror(err));
		return NULL;
	}
	if (smixer_level == 0 && (err = snd_mixer_attach(handle, card)) < 0) {
		error("Mixer attach %s error: %s", card, snd_strerror(err));
		snd_mixer_close(handle);
		handle = NULL;
		return NULL;
	}
	if ((err = snd_mixer_selem_register(handle, smixer_level > 0 ? &smixer_options : NULL, NULL)) < 0) {
		error("Mixer register error: %s", snd_strerror(err));
		snd_mixer_close(handle);
		handle = NULL;
		return NULL;
	}
	err = snd_mixer_load(handle);
	if (err < 0) {
		error("Mixer %s load error: %s", card, snd_strerror(err));
		snd_mixer_close(handle);
		handle = NULL;
		return NULL;
	}

	elem = snd_mixer_find_selem(handle, sid);
	if (!elem)
		return NULL;

	retval = get_selem(handle, sid);
	snd_mixer_close(handle);
	handle = NULL;
	return retval;
}

char* getAlsaVolume(char* device) {
	smixer_options.device = card;
	char* args[] = {device};

	return sset(args);
}

