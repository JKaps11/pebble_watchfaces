# The Studio reads shared code but never modifies it

The Studio (`studio/`) is a design sandbox: it borrows `shared/c/` and `shared/components/`
the same way a real watchface does, but nothing built there ships. The risk in that
arrangement is subtle — sooner or later a variant will want a small change to a shared
component, and making that edit to satisfy a throwaway design would put shipped code under
the influence of a sandbox. We therefore made the dependency strictly read-only: if a variant
needs a shared component to behave differently, it copies the component into `studio/` and
modifies the copy. A design exploration is never a reason to touch shipped code. The accepted
cost is duplication inside the Studio, which we consider harmless precisely because Studio
code is disposable; the rejected alternative — letting the Studio edit `shared/` — buys less
duplication at the price of the isolation that makes a sandbox worth having. Nothing under
`watchfaces/` may depend on `studio/` in any direction.

## Consequences

Repeated duplication of the same shared component across variants is a signal worth reading:
it suggests the component's real interface is wrong for more than one consumer. That is an
argument for changing `shared/components/` deliberately, as its own piece of work with its
own review — not as a side effect of a design session.
