_dragon_completion() {
    local cur prev shopts lopts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    shopts=( x t k p a A i T I s v )
    lopts=( and-exit target keep print-path all all-compact icon-only
        on-top stdin thumb-size verbose help version )

    if [[ "$cur" == --* ]]; then
        COMPREPLY=( $(compgen -W "${lopts[*]/#/--}" -- "$cur") )
        return 0
    fi

    if [[ "$cur" == -* ]]; then
        COMPREPLY=( $(compgen -W "${shopts[*]/#/-}" -- "$cur") )
        return 0
    fi

    compopt -o nospace 2>/dev/null
    COMPREPLY=( $(compgen -f -- "$cur") )
}

complete -F _dragon_completion dragon
