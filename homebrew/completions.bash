#!/usr/bin/env bash

_skad_updater_completions()
{
  local suggestions=$(skad_updater -h | grep -oe '--[A-Za-z\_]*')

  COMPREPLY=($(compgen -W "${suggestions[@]}" -- "${COMP_WORDS[1]}"))
}

complete -F _skad_updater_completions skad_updater