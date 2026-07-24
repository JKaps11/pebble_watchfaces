"""Command line for the Studio's tooling: `python3 -m studio <command>`."""

import argparse
import sys

from studio import batch, render, states


def _render(args):
    path = render.render(args.variant, args.state, args.output)
    print(path)


def _batch(args):
    result = batch.render_batch(
        args.session, args.variants,
        on_progress=lambda variant: print('rendered', variant, flush=True))
    print('contact sheet:', result['contact_sheet'])


def main(argv=None):
    parser = argparse.ArgumentParser(prog='studio', description=__doc__)
    commands = parser.add_subparsers(dest='command', required=True)

    render_command = commands.add_parser(
        'render', help='Render one Variant at one state.')
    render_command.add_argument('variant', help='Variant name, e.g. plain')
    render_command.add_argument('state', nargs='?', default='canonical',
                                choices=sorted(states.BY_NAME))
    render_command.add_argument('-o', '--output', default=None)
    render_command.set_defaults(handler=_render)

    batch_command = commands.add_parser(
        'batch', help='Render a Batch of six Variants and tile a Contact sheet.')
    batch_command.add_argument('session', help='Session name, e.g. 2026-07-24-mono')
    batch_command.add_argument('variants', nargs='+',
                               help='The six Variants in the Batch')
    batch_command.set_defaults(handler=_batch)

    args = parser.parse_args(argv)
    if args.command == 'render' and args.output is None:
        args.output = 'build/renders/{}_{}.png'.format(args.variant, args.state)

    try:
        args.handler(args)
    except (render.RenderError, batch.BatchError, KeyError) as error:
        print(error, file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
