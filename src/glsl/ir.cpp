/*
 * Copyright © 2010 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include <string.h>
#include "main/imports.h"
#include "ir.h"
#include "ir_visitor.h"
#include "glsl_types.h"

ir_assignment::ir_assignment(ir_rvalue *lhs, ir_rvalue *rhs,
			     ir_rvalue *condition)
{
   this->lhs = lhs;
   this->rhs = rhs;
   this->condition = condition;
}


ir_expression::ir_expression(int op, const struct glsl_type *type,
			     ir_rvalue *op0, ir_rvalue *op1)
{
   this->type = type;
   this->operation = ir_expression_operation(op);
   this->operands[0] = op0;
   this->operands[1] = op1;
}

unsigned int
ir_expression::get_num_operands(ir_expression_operation op)
{
/* Update ir_print_visitor.cpp when updating this list. */
   const int num_operands[] = {
      1, /* ir_unop_bit_not */
      1, /* ir_unop_logic_not */
      1, /* ir_unop_neg */
      1, /* ir_unop_abs */
      1, /* ir_unop_sign */
      1, /* ir_unop_rcp */
      1, /* ir_unop_rsq */
      1, /* ir_unop_sqrt */
      1, /* ir_unop_exp */
      1, /* ir_unop_log */
      1, /* ir_unop_exp2 */
      1, /* ir_unop_log2 */
      1, /* ir_unop_f2i */
      1, /* ir_unop_i2f */
      1, /* ir_unop_f2b */
      1, /* ir_unop_b2f */
      1, /* ir_unop_i2b */
      1, /* ir_unop_b2i */
      1, /* ir_unop_u2f */

      1, /* ir_unop_trunc */
      1, /* ir_unop_ceil */
      1, /* ir_unop_floor */

      1, /* ir_unop_sin */
      1, /* ir_unop_cos */

      1, /* ir_unop_dFdx */
      1, /* ir_unop_dFdy */

      2, /* ir_binop_add */
      2, /* ir_binop_sub */
      2, /* ir_binop_mul */
      2, /* ir_binop_div */
      2, /* ir_binop_mod */

      2, /* ir_binop_less */
      2, /* ir_binop_greater */
      2, /* ir_binop_lequal */
      2, /* ir_binop_gequal */
      2, /* ir_binop_equal */
      2, /* ir_binop_nequal */

      2, /* ir_binop_lshift */
      2, /* ir_binop_rshift */
      2, /* ir_binop_bit_and */
      2, /* ir_binop_bit_xor */
      2, /* ir_binop_bit_or */

      2, /* ir_binop_logic_and */
      2, /* ir_binop_logic_xor */
      2, /* ir_binop_logic_or */

      2, /* ir_binop_dot */
      2, /* ir_binop_min */
      2, /* ir_binop_max */

      2, /* ir_binop_pow */
   };

   assert(sizeof(num_operands) / sizeof(num_operands[0]) == ir_binop_pow + 1);

   return num_operands[op];
}

static const char *const operator_strs[] = {
   "~",
   "!",
   "neg",
   "abs",
   "sign",
   "rcp",
   "rsq",
   "sqrt",
   "exp",
   "log",
   "exp2",
   "log2",
   "f2i",
   "i2f",
   "f2b",
   "b2f",
   "i2b",
   "b2i",
   "u2f",
   "trunc",
   "ceil",
   "floor",
   "sin",
   "cos",
   "dFdx",
   "dFdy",
   "+",
   "-",
   "*",
   "/",
   "%",
   "<",
   ">",
   "<=",
   ">=",
   "==",
   "!=",
   "<<",
   ">>",
   "&",
   "^",
   "|",
   "&&",
   "^^",
   "||",
   "dot",
   "min",
   "max",
   "pow",
};

const char *ir_expression::operator_string()
{
   assert((unsigned int) operation <=
	  sizeof(operator_strs) / sizeof(operator_strs[0]));
   return operator_strs[operation];
}

ir_expression_operation
ir_expression::get_operator(const char *str)
{
   const int operator_count = sizeof(operator_strs) / sizeof(operator_strs[0]);
   for (int op = 0; op < operator_count; op++) {
      if (strcmp(str, operator_strs[op]) == 0)
	 return (ir_expression_operation) op;
   }
   return (ir_expression_operation) -1;
}

ir_constant::ir_constant()
{
   /* empty */
}

ir_constant::ir_constant(const struct glsl_type *type,
			 const ir_constant_data *data)
{
   assert((type->base_type >= GLSL_TYPE_UINT)
	  && (type->base_type <= GLSL_TYPE_BOOL));

   this->type = type;
   memcpy(& this->value, data, sizeof(this->value));
}

ir_constant::ir_constant(float f)
{
   this->type = glsl_type::float_type;
   this->value.f[0] = f;
}

ir_constant::ir_constant(unsigned int u)
{
   this->type = glsl_type::uint_type;
   this->value.u[0] = u;
}

ir_constant::ir_constant(int i)
{
   this->type = glsl_type::int_type;
   this->value.i[0] = i;
}

ir_constant::ir_constant(bool b)
{
   this->type = glsl_type::bool_type;
   this->value.b[0] = b;
}

ir_constant::ir_constant(const ir_constant *c, unsigned i)
{
   this->type = c->type->get_base_type();

   switch (this->type->base_type) {
   case GLSL_TYPE_UINT:  this->value.u[0] = c->value.u[i]; break;
   case GLSL_TYPE_INT:   this->value.i[0] = c->value.i[i]; break;
   case GLSL_TYPE_FLOAT: this->value.f[0] = c->value.f[i]; break;
   case GLSL_TYPE_BOOL:  this->value.b[0] = c->value.b[i]; break;
   default:              assert(!"Should not get here."); break;
   }
}

ir_constant::ir_constant(const struct glsl_type *type, exec_list *value_list)
{
   this->type = type;

   /* FINISHME: Support array types. */
   assert(type->is_scalar() || type->is_vector() || type->is_matrix()
	  || type->is_record());

   /* If the constant is a record, the types of each of the entries in
    * value_list must be a 1-for-1 match with the structure components.  Each
    * entry must also be a constant.  Just move the nodes from the value_list
    * to the list in the ir_constant.
    */
   /* FINISHME: Should there be some type checking and / or assertions here? */
   /* FINISHME: Should the new constant take ownership of the nodes from
    * FINISHME: value_list, or should it make copies?
    */
   if (type->is_record()) {
      value_list->move_nodes_to(& this->components);
      return;
   }


   ir_constant *value = (ir_constant *) (value_list->head);

   /* Use each component from each entry in the value_list to initialize one
    * component of the constant being constructed.
    */
   for (unsigned i = 0; i < type->components(); /* empty */) {
      assert(value->as_constant() != NULL);
      assert(!value->is_tail_sentinal());

      for (unsigned j = 0; j < value->type->components(); j++) {
	 switch (type->base_type) {
	 case GLSL_TYPE_UINT:
	    this->value.u[i] = value->get_uint_component(j);
	    break;
	 case GLSL_TYPE_INT:
	    this->value.i[i] = value->get_int_component(j);
	    break;
	 case GLSL_TYPE_FLOAT:
	    this->value.f[i] = value->get_float_component(j);
	    break;
	 case GLSL_TYPE_BOOL:
	    this->value.b[i] = value->get_bool_component(j);
	    break;
	 default:
	    /* FINISHME: What to do?  Exceptions are not the answer.
	     */
	    break;
	 }

	 i++;
	 if (i >= type->components())
	    break;
      }

      value = (ir_constant *) value->next;
   }
}

bool
ir_constant::get_bool_component(unsigned i) const
{
   switch (this->type->base_type) {
   case GLSL_TYPE_UINT:  return this->value.u[i] != 0;
   case GLSL_TYPE_INT:   return this->value.i[i] != 0;
   case GLSL_TYPE_FLOAT: return ((int)this->value.f[i]) != 0;
   case GLSL_TYPE_BOOL:  return this->value.b[i];
   default:              assert(!"Should not get here."); break;
   }

   /* Must return something to make the compiler happy.  This is clearly an
    * error case.
    */
   return false;
}

float
ir_constant::get_float_component(unsigned i) const
{
   switch (this->type->base_type) {
   case GLSL_TYPE_UINT:  return (float) this->value.u[i];
   case GLSL_TYPE_INT:   return (float) this->value.i[i];
   case GLSL_TYPE_FLOAT: return this->value.f[i];
   case GLSL_TYPE_BOOL:  return this->value.b[i] ? 1.0 : 0.0;
   default:              assert(!"Should not get here."); break;
   }

   /* Must return something to make the compiler happy.  This is clearly an
    * error case.
    */
   return 0.0;
}

int
ir_constant::get_int_component(unsigned i) const
{
   switch (this->type->base_type) {
   case GLSL_TYPE_UINT:  return this->value.u[i];
   case GLSL_TYPE_INT:   return this->value.i[i];
   case GLSL_TYPE_FLOAT: return (int) this->value.f[i];
   case GLSL_TYPE_BOOL:  return this->value.b[i] ? 1 : 0;
   default:              assert(!"Should not get here."); break;
   }

   /* Must return something to make the compiler happy.  This is clearly an
    * error case.
    */
   return 0;
}

unsigned
ir_constant::get_uint_component(unsigned i) const
{
   switch (this->type->base_type) {
   case GLSL_TYPE_UINT:  return this->value.u[i];
   case GLSL_TYPE_INT:   return this->value.i[i];
   case GLSL_TYPE_FLOAT: return (unsigned) this->value.f[i];
   case GLSL_TYPE_BOOL:  return this->value.b[i] ? 1 : 0;
   default:              assert(!"Should not get here."); break;
   }

   /* Must return something to make the compiler happy.  This is clearly an
    * error case.
    */
   return 0;
}


ir_constant *
ir_constant::get_record_field(const char *name)
{
   int idx = this->type->field_index(name);

   if (idx < 0)
      return NULL;

   if (this->components.is_empty())
      return NULL;

   exec_node *node = this->components.head;
   for (int i = 0; i < idx; i++) {
      node = node->next;

      /* If the end of the list is encountered before the element matching the
       * requested field is found, return NULL.
       */
      if (node->is_tail_sentinal())
	 return NULL;
   }

   return (ir_constant *) node;
}


bool
ir_constant::has_value(const ir_constant *c) const
{
   if (this->type != c->type)
      return false;

   /* FINISHME: This will probably also handle constant arrays as soon as those
    * FINISHME: are supported.
    */
   if (this->type->base_type == GLSL_TYPE_STRUCT) {
      const exec_node *a_node = this->components.head;
      const exec_node *b_node = c->components.head;

      while (!a_node->is_tail_sentinal()) {
	 assert(!b_node->is_tail_sentinal());

	 const ir_constant *const a_field = (ir_constant *) a_node;
	 const ir_constant *const b_field = (ir_constant *) b_node;

	 if (!a_field->has_value(b_field))
	    return false;

	 a_node = a_node->next;
	 b_node = b_node->next;
      }

      return true;
   }

   for (unsigned i = 0; i < this->type->components(); i++) {
      switch (this->type->base_type) {
      case GLSL_TYPE_UINT:
	 if (this->value.u[i] != c->value.u[i])
	    return false;
	 break;
      case GLSL_TYPE_INT:
	 if (this->value.i[i] != c->value.i[i])
	    return false;
	 break;
      case GLSL_TYPE_FLOAT:
	 if (this->value.f[i] != c->value.f[i])
	    return false;
	 break;
      case GLSL_TYPE_BOOL:
	 if (this->value.b[i] != c->value.b[i])
	    return false;
	 break;
      default:
	 assert(!"Should not get here.");
	 return false;
      }
   }

   return true;
}

ir_dereference_variable::ir_dereference_variable(ir_variable *var)
{
   this->var = var;
   this->type = (var != NULL) ? var->type : glsl_type::error_type;
}


ir_dereference_array::ir_dereference_array(ir_rvalue *value,
					   ir_rvalue *array_index)
{
   this->array_index = array_index;
   this->set_array(value);
}


ir_dereference_array::ir_dereference_array(ir_variable *var,
					   ir_rvalue *array_index)
{
   void *ctx = talloc_parent(var);

   this->array_index = array_index;
   this->set_array(new(ctx) ir_dereference_variable(var));
}


void
ir_dereference_array::set_array(ir_rvalue *value)
{
   this->array = value;
   this->type = glsl_type::error_type;

   if (this->array != NULL) {
      const glsl_type *const vt = this->array->type;

      if (vt->is_array()) {
	 type = vt->element_type();
      } else if (vt->is_matrix()) {
	 type = vt->column_type();
      } else if (vt->is_vector()) {
	 type = vt->get_base_type();
      }
   }
}


ir_dereference_record::ir_dereference_record(ir_rvalue *value,
					     const char *field)
{
   this->record = value;
   this->field = field;
   this->type = (this->record != NULL)
      ? this->record->type->field_type(field) : glsl_type::error_type;
}


ir_dereference_record::ir_dereference_record(ir_variable *var,
					     const char *field)
{
   void *ctx = talloc_parent(var);

   this->record = new(ctx) ir_dereference_variable(var);
   this->field = field;
   this->type = (this->record != NULL)
      ? this->record->type->field_type(field) : glsl_type::error_type;
}


bool
ir_dereference::is_lvalue()
{
   ir_variable *var = this->variable_referenced();

   /* Every l-value derference chain eventually ends in a variable.
    */
   if ((var == NULL) || var->read_only)
      return false;

   if (this->type->is_array() && !var->array_lvalue)
      return false;

   return true;
}


const char *tex_opcode_strs[] = { "tex", "txb", "txl", "txd", "txf" };

const char *ir_texture::opcode_string()
{
   assert((unsigned int) op <=
	  sizeof(tex_opcode_strs) / sizeof(tex_opcode_strs[0]));
   return tex_opcode_strs[op];
}

ir_texture_opcode
ir_texture::get_opcode(const char *str)
{
   const int count = sizeof(tex_opcode_strs) / sizeof(tex_opcode_strs[0]);
   for (int op = 0; op < count; op++) {
      if (strcmp(str, tex_opcode_strs[op]) == 0)
	 return (ir_texture_opcode) op;
   }
   return (ir_texture_opcode) -1;
}


void
ir_texture::set_sampler(ir_dereference *sampler)
{
   assert(sampler != NULL);
   this->sampler = sampler;

   switch (sampler->type->sampler_type) {
   case GLSL_TYPE_FLOAT:
      this->type = glsl_type::vec4_type;
      break;
   case GLSL_TYPE_INT:
      this->type = glsl_type::ivec4_type;
      break;
   case GLSL_TYPE_UINT:
      this->type = glsl_type::uvec4_type;
      break;
   }
}


ir_swizzle::ir_swizzle(ir_rvalue *val, unsigned x, unsigned y, unsigned z,
		       unsigned w, unsigned count)
   : val(val)
{
   assert((count >= 1) && (count <= 4));

   const unsigned dup_mask = 0
      | ((count > 1) ? ((1U << y) & ((1U << x)                        )) : 0)
      | ((count > 2) ? ((1U << z) & ((1U << x) | (1U << y)            )) : 0)
      | ((count > 3) ? ((1U << w) & ((1U << x) | (1U << y) | (1U << z))) : 0);

   assert(x <= 3);
   assert(y <= 3);
   assert(z <= 3);
   assert(w <= 3);

   mask.x = x;
   mask.y = y;
   mask.z = z;
   mask.w = w;
   mask.num_components = count;
   mask.has_duplicates = dup_mask != 0;

   /* Based on the number of elements in the swizzle and the base type
    * (i.e., float, int, unsigned, or bool) of the vector being swizzled,
    * generate the type of the resulting value.
    */
   type = glsl_type::get_instance(val->type->base_type, mask.num_components, 1);
}

ir_swizzle::ir_swizzle(ir_rvalue *val, ir_swizzle_mask mask)
{
   this->val = val;
   this->mask = mask;
   this->type = glsl_type::get_instance(val->type->base_type,
					mask.num_components, 1);
}

#define X 1
#define R 5
#define S 9
#define I 13

ir_swizzle *
ir_swizzle::create(ir_rvalue *val, const char *str, unsigned vector_length)
{
   void *ctx = talloc_parent(val);

   /* For each possible swizzle character, this table encodes the value in
    * \c idx_map that represents the 0th element of the vector.  For invalid
    * swizzle characters (e.g., 'k'), a special value is used that will allow
    * detection of errors.
    */
   static const unsigned char base_idx[26] = {
   /* a  b  c  d  e  f  g  h  i  j  k  l  m */
      R, R, I, I, I, I, R, I, I, I, I, I, I,
   /* n  o  p  q  r  s  t  u  v  w  x  y  z */
      I, I, S, S, R, S, S, I, I, X, X, X, X
   };

   /* Each valid swizzle character has an entry in the previous table.  This
    * table encodes the base index encoded in the previous table plus the actual
    * index of the swizzle character.  When processing swizzles, the first
    * character in the string is indexed in the previous table.  Each character
    * in the string is indexed in this table, and the value found there has the
    * value form the first table subtracted.  The result must be on the range
    * [0,3].
    *
    * For example, the string "wzyx" will get X from the first table.  Each of
    * the charcaters will get X+3, X+2, X+1, and X+0 from this table.  After
    * subtraction, the swizzle values are { 3, 2, 1, 0 }.
    *
    * The string "wzrg" will get X from the first table.  Each of the characters
    * will get X+3, X+2, R+0, and R+1 from this table.  After subtraction, the
    * swizzle values are { 3, 2, 4, 5 }.  Since 4 and 5 are outside the range
    * [0,3], the error is detected.
    */
   static const unsigned char idx_map[26] = {
   /* a    b    c    d    e    f    g    h    i    j    k    l    m */
      R+3, R+2, 0,   0,   0,   0,   R+1, 0,   0,   0,   0,   0,   0,
   /* n    o    p    q    r    s    t    u    v    w    x    y    z */
      0,   0,   S+2, S+3, R+0, S+0, S+1, 0,   0,   X+3, X+0, X+1, X+2
   };

   int swiz_idx[4] = { 0, 0, 0, 0 };
   unsigned i;


   /* Validate the first character in the swizzle string and look up the base
    * index value as described above.
    */
   if ((str[0] < 'a') || (str[0] > 'z'))
      return NULL;

   const unsigned base = base_idx[str[0] - 'a'];


   for (i = 0; (i < 4) && (str[i] != '\0'); i++) {
      /* Validate the next character, and, as described above, convert it to a
       * swizzle index.
       */
      if ((str[i] < 'a') || (str[i] > 'z'))
	 return NULL;

      swiz_idx[i] = idx_map[str[i] - 'a'] - base;
      if ((swiz_idx[i] < 0) || (swiz_idx[i] >= (int) vector_length))
	 return NULL;
   }

   if (str[i] != '\0')
	 return NULL;

   return new(ctx) ir_swizzle(val, swiz_idx[0], swiz_idx[1], swiz_idx[2],
			      swiz_idx[3], i);
}

#undef X
#undef R
#undef S
#undef I

ir_variable *
ir_swizzle::variable_referenced()
{
   return this->val->variable_referenced();
}

ir_variable::ir_variable(const struct glsl_type *type, const char *name)
   : max_array_access(0), read_only(false), centroid(false), invariant(false),
     shader_in(false), shader_out(false),
     mode(ir_var_auto), interpolation(ir_var_smooth), array_lvalue(false)
{
   this->type = type;
   this->name = name;
   this->location = -1;
   this->warn_extension = NULL;
   this->constant_value = NULL;

   if (type && type->base_type == GLSL_TYPE_SAMPLER)
      this->read_only = true;
}


const char *
ir_variable::interpolation_string() const
{
   if (!this->shader_in && !this->shader_out)
      return "";

   switch (this->interpolation) {
   case ir_var_smooth:        return "smooth";
   case ir_var_flat:          return "flat";
   case ir_var_noperspective: return "noperspective";
   }

   assert(!"Should not get here.");
   return "";
}


unsigned
ir_variable::component_slots() const
{
   /* FINISHME: Sparsely accessed arrays require fewer slots. */
   return this->type->component_slots();
}


ir_function_signature::ir_function_signature(const glsl_type *return_type)
   : return_type(return_type), is_defined(false)
{
   /* empty */
}


const char *
ir_function_signature::qualifiers_match(exec_list *params)
{
   exec_list_iterator iter_a = parameters.iterator();
   exec_list_iterator iter_b = params->iterator();

   /* check that the qualifiers match. */
   while (iter_a.has_next()) {
      ir_variable *a = (ir_variable *)iter_a.get();
      ir_variable *b = (ir_variable *)iter_b.get();

      if (a->read_only != b->read_only ||
	  a->mode != b->mode ||
	  a->interpolation != b->interpolation ||
	  a->centroid != b->centroid) {

	 /* parameter a's qualifiers don't match */
	 return a->name;
      }

      iter_a.next();
      iter_b.next();
   }
   return NULL;
}


void
ir_function_signature::replace_parameters(exec_list *new_params)
{
   /* Destroy all of the previous parameter information.  If the previous
    * parameter information comes from the function prototype, it may either
    * specify incorrect parameter names or not have names at all.
    */
   foreach_iter(exec_list_iterator, iter, parameters) {
      assert(((ir_instruction *) iter.get())->as_variable() != NULL);

      iter.remove();
   }

   new_params->move_nodes_to(&parameters);
}


ir_function::ir_function(const char *name)
   : name(name)
{
   /* empty */
}


ir_call *
ir_call::get_error_instruction(void *ctx)
{
   ir_call *call = new(ctx) ir_call;

   call->type = glsl_type::error_type;
   return call;
}

void
visit_exec_list(exec_list *list, ir_visitor *visitor)
{
   foreach_iter(exec_list_iterator, iter, *list) {
      ((ir_instruction *)iter.get())->accept(visitor);
   }
}
